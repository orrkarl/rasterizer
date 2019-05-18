#pragma once

#include "../general/predefs.h"

namespace nr
{

namespace __internal
{

namespace clcode
{

const string base = R"__CODE__(

// -------------------------------------- Types -------------------------------------- 

#ifndef RENDER_DIMENSION 
    #error "RENDER_DIMENSION has to be defined!"
#endif

#if RENDER_DIMENSION < 3
    #error "RENDER_DIMENSION has to be at least 3!"
#endif

#ifndef MAX_WORK_GROUP_COUNT
    #define MAX_WORK_GROUP_COUNT (16)
#endif

typedef struct _ScreenDimension
{
    uint width;
    uint height;
} ScreenDimension;

typedef uint2 ScreenPosition;

typedef int2 SignedScreenPosition;

typedef float2 NDCPosition;

typedef float3 ColorRGB;

typedef float4 ColorRGBA;

typedef uint  Index;
typedef float Depth;

typedef struct _RawColorRGB
{
    uchar r, g, b;
} RawColorRGB;

typedef struct _FrameBuffer
{
    RawColorRGB*    color;
    Depth*          depth;
} FrameBuffer;

typedef struct _Fragment
{
    ScreenPosition position;
    RawColorRGB color;
    Depth depth;
} Fragment;

typedef struct _Bin
{
    uint width;
    uint height;
    uint x;
    uint y;
} Bin;

typedef float Point[RENDER_DIMENSION];   // point in n-dimensional space 
typedef Point Triangle[3];               // Nth dimensional triangle
typedef Point Simplex[RENDER_DIMENSION]; // N-1 simplex (rendering is done on an object's surface)

typedef struct _BinQueueConfig
{
    uint bin_width;
    uint bin_height;
    uint queue_size;
} BinQueueConfig;

// ----------------------------------------------------------------------------

// -------------------------------------- Globals -------------------------------------- 

#define RAW_RED ((RawColorRGB) {255, 0, 0})

#define r(vec) (vec.x)
#define g(vec) (vec.y)
#define b(vec) (vec.z)

// ----------------------------------------------------------------------------


// -------------------------------------- Utilities -------------------------------------- 

uint index_from_screen(const ScreenPosition pos, const ScreenDimension dim)
{
    return pos.y * dim.width + pos.x;
}

uint from_continuous(const float continuous)
{
    return floor(continuous);
}

float from_discrete(const uint discrete)
{
    return discrete + 0.5;
}

uint axis_screen_from_ndc(const float pos, const uint length)
{
    return round((float) ((pos + 1.0) * (length - 1) * 0.5));
}

float axis_ndc_from_screen(const uint pos, const uint length)
{
    return ((float) pos) * 2.0 / (length - 1) - 1.0;
}

void screen_from_ndc(const NDCPosition ndc, const ScreenDimension dim, ScreenPosition* screen)
{
    screen->x = axis_screen_from_ndc(ndc.x, dim.width);
    screen->y = axis_screen_from_ndc(ndc.y, dim.height);
}

int axis_signed_from_ndc(const float pos, const uint length)
{
    return axis_screen_from_ndc(pos, length) - length / 2;
}

void signed_from_ndc(const NDCPosition ndc, const ScreenDimension dim, SignedScreenPosition* screen)
{
    screen->x = axis_signed_from_ndc(ndc.x, dim.width);
    screen->y = axis_signed_from_ndc(ndc.y, dim.height);
}

void screen_from_signed(const SignedScreenPosition pos, const ScreenDimension dim, ScreenPosition* screen)
{
    screen->x = pos.x + dim.width / 2;
    screen->y = pos.y + dim.height / 2;
}

void ndc_from_screen(const ScreenPosition screen, const ScreenDimension dim, NDCPosition* result)
{
    result->x = axis_ndc_from_screen(screen.x, dim.width);
    result->y = axis_ndc_from_screen(screen.y, dim.height);
}

// ----------------------------------------------------------------------------

// -------------------------------------- Debugging -------------------------------------- 

#ifdef _DEBUG
    #define DEBUG(code) code
#else
    #define DEBUG(code)
#endif // _DEBUG

#define IS_WORK_ITEM_GLOBAL(i, j, k) (get_global_id(0) == i && get_global_id(1) == j && get_global_id(2) == k)
#define IS_WORK_ITEM_LOCAL(i, j, k) (get_local_id(0) == i && get_local_id(1) == j && get_local_id(2) == k)

#define IS_GROUP_HEAD IS_WORK_ITEM_LOCAL(0, 0, 0)

#define DEBUG_MESSAGE(msg)                                              DEBUG(printf(msg))
#define DEBUG_MESSAGE1(msg, arg1)                                       DEBUG(printf(msg, arg1))
#define DEBUG_MESSAGE2(msg, arg1, arg2)                                 DEBUG(printf(msg, arg1, arg2))
#define DEBUG_MESSAGE3(msg, arg1, arg2, arg3)                           DEBUG(printf(msg, arg1, arg2, arg3))
#define DEBUG_MESSAGE4(msg, arg1, arg2, arg3, arg4)                     DEBUG(printf(msg, arg1, arg2, arg3, arg4))
#define DEBUG_MESSAGE5(msg, arg1, arg2, arg3, arg4, arg5)               DEBUG(printf(msg, arg1, arg2, arg3, arg4, arg5))
#define DEBUG_MESSAGE6(msg, arg1, arg2, arg3, arg4, arg5, arg6)         DEBUG(printf(msg, arg1, arg2, arg3, arg4, arg5, arg6))
#define DEBUG_MESSAGE7(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7)   DEBUG(printf(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7))

#define DEBUG_ITEM_SPECIFIC(i, j, k, msg) DEBUG(if (IS_WORK_ITEM_GLOBAL(i, j, k)) { printf(msg); } else {})
#define DEBUG_ITEM_SPECIFIC1(i, j, k, msg, arg1) DEBUG(if (IS_WORK_ITEM_GLOBAL(i, j, k)) { printf(msg, arg1); } else {})
#define DEBUG_ITEM_SPECIFIC2(i, j, k, msg, arg1, arg2) DEBUG(if (IS_WORK_ITEM_GLOBAL(i, j, k)) { printf(msg, arg1, arg2); } else {})
#define DEBUG_ITEM_SPECIFIC3(i, j, k, msg, arg1, arg2, arg3) DEBUG(if (IS_WORK_ITEM_GLOBAL(i, j, k)) { printf(msg, arg1, arg2, arg3); } else {})
#define DEBUG_ITEM_SPECIFIC4(i, j, k, msg, arg1, arg2, arg3, arg4) DEBUG(if (IS_WORK_ITEM_GLOBAL(i, j, k)) { printf(msg, arg1, arg2, arg3, arg4); } else {})
#define DEBUG_ITEM_SPECIFIC5(i, j, k, msg, arg1, arg2, arg3, arg4, arg5) DEBUG(if (IS_WORK_ITEM_GLOBAL(i, j, k)) { printf(msg, arg1, arg2, arg3, arg4, arg5); } else {})
#define DEBUG_ITEM_SPECIFIC6(i, j, k, msg, arg1, arg2, arg3, arg4, arg5, arg6) DEBUG(if (IS_WORK_ITEM_GLOBAL(i, j, k)) { printf(msg, arg1, arg2, arg3, arg4, arg5, arg6); } else {})
#define DEBUG_ITEM_SPECIFIC7(i, j, k, msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7) DEBUG(if (IS_WORK_ITEM_GLOBAL(i, j, k)) { printf(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7); } else {})
#define DEBUG_ITEM_SPECIFIC8(i, j, k, msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) DEBUG(if (IS_WORK_ITEM_GLOBAL(i, j, k)) { printf(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8); } else {})

// Prints only from the first work item in a work group
// Prints once PER GROUP
#define DEBUG_ONCE(msg)                                             DEBUG(if (IS_GROUP_HEAD) { printf(msg); } else {})
#define DEBUG_ONCE1(msg, arg1)                                      DEBUG(if (IS_GROUP_HEAD) { printf(msg, arg1); } else {}) 
#define DEBUG_ONCE2(msg, arg1, arg2)                                DEBUG(if (IS_GROUP_HEAD) { printf(msg, arg1, arg2); } else {})
#define DEBUG_ONCE3(msg, arg1, arg2, arg3)                          DEBUG(if (IS_GROUP_HEAD) { printf(msg, arg1, arg2, arg3); } else {})
#define DEBUG_ONCE4(msg, arg1, arg2, arg3, arg4)                    DEBUG(if (IS_GROUP_HEAD) { printf(msg, arg1, arg2, arg3, arg4); } else {})
#define DEBUG_ONCE5(msg, arg1, arg2, arg3, arg4, arg5)              DEBUG(if (IS_GROUP_HEAD) { printf(msg, arg1, arg2, arg3, arg4, arg5); } else {})
#define DEBUG_ONCE6(msg, arg1, arg2, arg3, arg4, arg5, arg6)        DEBUG(if (IS_GROUP_HEAD) { printf(msg, arg1, arg2, arg3, arg4, arg5, arg6); } else {})
#define DEBUG_ONCE7(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7)  DEBUG(if (IS_GROUP_HEAD) { printf(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7); } else {})
#define DEBUG_ONCE8(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)  DEBUG(if (IS_GROUP_HEAD) { printf(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8); } else {})

// ----------------------------------------------------------------------------

// -------------------------------------- Testing -------------------------------------- 

kernel void screen_from_ndc_kernel(NDCPosition pos, ScreenDimension dim, global ScreenPosition* res)
{
    screen_from_ndc(pos, dim, res);
}

kernel void ndc_from_screen_test(ScreenPosition pos, ScreenDimension dim, global NDCPosition* res)
{
    ndc_from_screen(pos, dim, res);
}

)__CODE__";

}

}

}