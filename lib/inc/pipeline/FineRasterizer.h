#pragma once

#include "../general/predefs.h"
#include "../base/Buffer.h"
#include "../base/Kernel.h"
#include "../rendering/Render.h"
#include "BinRasterizer.h"

namespace nr
{

namespace __internal
{

class FineRasterizerParams
{
public:
    cl_int init(cl::CommandQueue q) { return CL_SUCCESS; }

    cl_int load(cl::Kernel kernel);

    // Simplex buffer
    Buffer simplexData;

    // Screen Dimensions [size in pixels]
    ScreenDimension dim;

    // Bin Queues
    BinQueueConfig binQueueConfig;
    Buffer binQueues;

    // Overflow marker
    Buffer overflow;

    // Frame buffer, pretty intuitive
    FrameBuffer frameBuffer;
};

typedef Kernel<FineRasterizerParams> FineRasterizer;

}

}