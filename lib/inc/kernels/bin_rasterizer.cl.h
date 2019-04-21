#pragma once

#include "../general/predefs.h"

namespace nr
{

namespace __internal
{

namespace clcode
{

const string bin_rasterizer = R"__CODE__(

// -------------------------------------- Types and defines -------------------------------------- 

// DON'T CHANGE THIS 
#define BATCH_COUNT (256)

typedef struct _Bin
{
    uint width;
    uint height;
    uint x;
    uint y;
} Bin;


// ----------------------------------------------------------------------------

// -------------------------------------- Utilities --------------------------------------

bool is_point_in_bin(const uint x, const uint y, const Bin bin)
{
    return bin.x <= x && x < bin.x + bin.width && bin.y <= y && y < bin.y + bin.height; 
}

bool is_simplex_in_bin(const generic float x[RENDER_DIMENSION], const generic float y[RENDER_DIMENSION], const Bin bin, const ScreenDimension dim)
{
    bool result = false;
    uint curr_x, curr_y;

    __attribute__((opencl_unroll_hint))
    for (uint j = 0; j < RENDER_DIMENSION; ++j)
    {
        curr_x = axis_screen_from_ndc(x[j], dim.width);
        curr_y = axis_screen_from_ndc(y[j], dim.height);
        result |= is_point_in_bin(curr_x, curr_y, bin);
    }
    return result;
}

Bin make_bin(const ScreenDimension dim, const uint index_x, const uint index_y, const uint bin_width, const uint bin_height)
{
    Bin ret;
    ret.x = index_x * bin_width;
    ret.y = index_y * bin_height;
    ret.width = min(dim.width - ret.x, bin_width);
    ret.height = min(dim.height - ret.y, bin_height); 
    return ret;
}

event_t reduce_simplex_buffer(
    const global Simplex* simplices, 
    const uint simplex_count, 
    const uint offset, 
    event_t event, 
    local float* result_x, 
    local float* result_y)
{
    local float* dest_x = (local float*) result_x;
    local float* dest_y = (local float*) result_y;
    const global float* src_base = ((const global float*) simplices) + offset; 

    const uint raw_copy_count = simplex_count * RENDER_DIMENSION; // one for each point in the simplex buffer

    event_t ret = async_work_group_strided_copy(dest_x, src_base, raw_copy_count, RENDER_DIMENSION, 0);  // Copying x values
    return async_work_group_strided_copy(dest_y, src_base + 1, raw_copy_count, RENDER_DIMENSION, ret);   // Copying y values
}

// ----------------------------------------------------------------------------

global atomic_uint g_batch_index;

kernel void bin_rasterize(
    const global Simplex* simplex_data,
    const uint simplex_count,
    const ScreenDimension dim,
    const BinQueueConfig config,
    global bool* has_overflow,
    global Index* bin_queues)
{
    local float reduced_simplices_x[BATCH_COUNT * RENDER_DIMENSION];
    local float reduced_simplices_y[BATCH_COUNT * RENDER_DIMENSION];
    local uint current_batch_index;

    // Workaround for that wierd compiler bug
    private const ScreenDimension screenDim = dim;
    
    private uint index_x = get_local_id(0);
    private uint index_y = get_local_id(1);

    private bool is_init_manager = !index_x && !index_y;

    private event_t batch_acquisition = 0;
    
    private uint bin_queue_index = 0;
    private const uint bins_count_x = ceil(((float) screenDim.width) / config.bin_width);
    private const uint bins_count_y = ceil(((float) screenDim.height) / config.bin_height);
    private uint bin_queue_base = config.queue_size * (bins_count_x * bins_count_y * (get_group_id(1) * get_num_groups(0) + get_group_id(0)) + bins_count_x * index_y + index_x); 
    private uint current_queue_index  = bin_queue_base + 1;

    private uint batch_actual_size;

    private const Bin current_bin = make_bin(screenDim, index_x, index_y, config.bin_width, config.bin_height);

    if (!get_global_id(0) && !get_global_id(1))
    {
        atomic_init(&g_batch_index, 0);
        *has_overflow = false;
    }

    if (is_init_manager)
    {
        current_batch_index = 0;
    }

    // wait for LOCAL batch index initialization
    work_group_barrier(CLK_LOCAL_MEM_FENCE);

    // wait for GLOBAL batch index initialization
    work_group_barrier(CLK_GLOBAL_MEM_FENCE);

    bin_queues[bin_queue_base] = 1;

    while (true)
    {
        work_group_barrier(CLK_LOCAL_MEM_FENCE);

        // Aquire a batch (update the local and global batch index)
        if (is_init_manager)
        {
            current_batch_index = atomic_fetch_add(&g_batch_index, BATCH_COUNT);
        }

        work_group_barrier(CLK_LOCAL_MEM_FENCE);

        if (*has_overflow)
        {
            return;
        }
        
        // no more batches to process, this work group has no more work to do
        if (current_batch_index >= simplex_count)
        {
            return;
        }
        
        batch_actual_size = min((uint) BATCH_COUNT, simplex_count - current_batch_index);

        // Copying x values of each point
        batch_acquisition = reduce_simplex_buffer(simplex_data, batch_actual_size, current_batch_index, 0, reduced_simplices_x, reduced_simplices_y);
        wait_group_events(1, &batch_acquisition);

        for (private uint i = 0; i < batch_actual_size; ++i)
        {
            if (
                is_simplex_in_bin(
                    reduced_simplices_x + i * RENDER_DIMENSION, 
                    reduced_simplices_y + i * RENDER_DIMENSION, 
                    current_bin, 
                    screenDim))
            {   
                bin_queues[current_queue_index++] = current_batch_index + i;
                bin_queues[bin_queue_base] = 0;
            }

            // An overflowing queue was detected
            if (current_queue_index >= config.queue_size + bin_queue_base + 1)
            {
                *has_overflow = true;
                break;
            }
        }

        if (current_queue_index != bin_queue_base + 1 + config.queue_size)
        {
            bin_queues[current_queue_index] = 0;
        }
    }     
}

// -------------------------------------- Tests --------------------------------------

#ifdef _TEST_BINNING

#ifndef SIMPLEX_TEST_COUNT
    #error "SIMPLEX_TEST_COUNT has to be defined in binning test configuration"
#endif 

kernel void is_simplex_in_bin_test(
    const global float simplex_x[RENDER_DIMENSION],
    const global float simplex_y[RENDER_DIMENSION], 
    const Bin bin,
    const ScreenDimension dim,
    global bool* result)
{
    *result = is_simplex_in_bin(simplex_x, simplex_y, bin, dim);
}

kernel void reduce_simplex_buffer_test(
    const global Simplex simplex_data[SIMPLEX_TEST_COUNT],
    const uint offset,
    global NDCPosition* result)
{
    local float res_x[SIMPLEX_TEST_COUNT * RENDER_DIMENSION];
    local float res_y[SIMPLEX_TEST_COUNT * RENDER_DIMENSION];
    event_t wait = reduce_simplex_buffer(simplex_data, SIMPLEX_TEST_COUNT, offset, 0, res_x, res_y);
    wait_group_events(1, &wait);

    if (get_global_id(0) == 0)
    {
        __attribute__((opencl_unroll_hint))
        for (uint i = 0; i < SIMPLEX_TEST_COUNT * RENDER_DIMENSION; ++i)
        {
            result[i].x = res_x[i];
            result[i].y = res_y[i];
        }
    }
}

#endif // _TEST_BINNING

)__CODE__";

}

}

}