#pragma once

#include "../general/predefs.h"

#include "../base/Buffer.h"
#include "../base/CommandQueue.h"
#include "../base/Kernel.h"
#include "../base/Module.h"

#include "../rendering/Render.h"

namespace nr
{

namespace detail
{

struct NRAPI SimplexReducer : Kernel
{
	static nr_uint getTriangleCount(const nr_uint simplexCount);

	SimplexReducer(Module module, cl_status* err = nullptr);

	SimplexReducer();

	cl_status load();

	Buffer simplexes;
	Buffer result;
};

}

}