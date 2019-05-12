#pragma once

#include <inc/includes.h>

#include "bin_utils.h"

#include <base/Module.h>
#include <kernels/base.cl.h>
#include <kernels/bin_rasterizer.cl.h>
#include <pipeline/BinRasterizer.h>
#include <base/Buffer.h>

#include <chrono>

using namespace nr;
using namespace nr::__internal;
using namespace testing;

TEST(Binning, Rasterizer)
{
    cl_int err = CL_SUCCESS; 
    Error error = Error::NO_ERROR;

    const NRuint dim = 4;
    const NRuint simplexCount = 2;

    // Don't change this, may be critical for the test's determinism
    const NRuint workGroupCount = 1;

    const ScreenDimension screenDim = {64, 64};
    const BinQueueConfig config = {32, 32, 10};

    NRuint h_result[BinRasterizerParams::getTotalBinQueueSize(workGroupCount, screenDim, config) / sizeof(NRuint)];

    NRbool isOverflowing = false;

    const NRuint binCountX = ceil(((NRfloat) screenDim.width) / config.binWidth);
    const NRuint binCountY = ceil(((NRfloat) screenDim.height) / config.binHeight);
    const NRuint destinationBinX = 1;
    const NRuint destinationBinY = 1;
    const NRuint destinationBinQueueRawIndex = config.queueSize * (destinationBinY * binCountX + destinationBinX);
    const NRuint* destBinQueueBase = h_result + destinationBinQueueRawIndex;

    const NRuint x = destinationBinX * config.binWidth + config.binWidth / 2;
    const NRuint y = destinationBinY * config.binHeight + config.binHeight / 2;
    Simplex<dim> h_simplices[simplexCount];
    mkSimplexInCoords(x, y, screenDim, h_simplices);
    mkSimplexInCoords(x, y, screenDim, h_simplices + 1);

    const char options_fmt[] = "-cl-std=CL2.0 -Werror -D _DEBUG -D _TEST_BINNING -D RENDER_DIMENSION=%d -D SIMPLEX_TEST_COUNT=%d";
    char options[sizeof(options_fmt) * 2];
    memset(options, 0, sizeof(options));

    sprintf(options, options_fmt, dim, simplexCount);

    cl::CommandQueue q = cl::CommandQueue::getDefault();

    auto code = Module({clcode::base, clcode::bin_rasterizer}, options, &err);
    ASSERT_EQ(CL_SUCCESS, err);

    auto testee = code.makeKernel<BinRasterizerParams>("bin_rasterize", &err);
    ASSERT_EQ(CL_SUCCESS, err);

    Buffer d_simplices(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, simplexCount * sizeof(Simplex<dim>), (NRfloat*) h_simplices, &error);
    ASSERT_PRED1(error::isSuccess, error);

    Buffer d_overflow(CL_MEM_READ_WRITE, sizeof(cl_bool), nullptr, &error);
    ASSERT_PRED1(error::isSuccess, error);

    Buffer d_binQueues(CL_MEM_READ_WRITE, BinRasterizerParams::getTotalBinQueueSize(workGroupCount, screenDim, config), &error);
    ASSERT_PRED1(error::isSuccess, error);

    testee.params.binQueueConfig = config;
    testee.params.dimension = screenDim;
    testee.params.simplexData = d_simplices;
    testee.params.simplexCount = simplexCount;
    testee.params.hasOverflow = d_overflow;
    testee.params.binQueues = d_binQueues;

    testee.global = cl::NDRange(workGroupCount * binCountX, binCountY);
    testee.local  = cl::NDRange(binCountX, binCountY);

    ASSERT_EQ(CL_SUCCESS, testee(q));
    ASSERT_EQ(CL_SUCCESS, q.enqueueReadBuffer(d_binQueues, CL_FALSE, 0, sizeof(h_result), h_result));
    ASSERT_EQ(CL_SUCCESS, q.enqueueReadBuffer(d_overflow, CL_FALSE, 0, sizeof(isOverflowing), &isOverflowing));
    ASSERT_EQ(CL_SUCCESS, q.finish());
    ASSERT_FALSE(isOverflowing);
    ASSERT_EQ(CL_SUCCESS, err);

    ASSERT_EQ(0, h_result[0]);  // queue for bin (0, 0) is not empty
    ASSERT_EQ(0, h_result[1]);  // the first simplex is in it
    ASSERT_EQ(1, h_result[2]);  // the second simplex is in it
    ASSERT_EQ(0, h_result[3]);  // no more simplices

    ASSERT_EQ(0, destBinQueueBase[0]); // the designated queue is not empty  
    ASSERT_EQ(0, destBinQueueBase[1]); // the queue's first element points to the first simplex
    ASSERT_EQ(1, destBinQueueBase[2]); // the queue's second element points to the second simplex
    ASSERT_EQ(0, destBinQueueBase[3]); // queue end
}