#include "rendermath.h"

#include <cmath>

namespace nr
{

namespace detail
{

nr_uint triangleCount(const nr_uint dim, const nr_uint simplexCount)
{
	return dim * (dim - 1) * (dim - 2) * simplexCount / 6;
}

std::pair<nr_uint, nr_uint> getBinCount(const ScreenDimension& screenDim, const BinQueueConfig& config)
{
	return { static_cast<nr_uint>(std::ceil(nr_float(screenDim.width) / config.binWidth)), static_cast<nr_uint>(std::ceil(nr_float(screenDim.height) / config.binHeight)) };
}

}

}