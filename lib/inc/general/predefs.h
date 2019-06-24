#pragma once

#include <cstdint>
#include <string>
#include <cmath>
#include <stdio.h>

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>

typedef double nr_double;
typedef float nr_float;

typedef int64_t nr_long;
typedef uint64_t nr_ulong;

typedef int32_t nr_int;
typedef uint32_t nr_uint;

typedef int16_t nr_short;
typedef uint16_t nr_ushort;

typedef int8_t nr_byte;
typedef uint8_t nr_ubyte;

typedef char nr_char;
typedef bool nr_bool;

typedef cl_int cl_status;

typedef size_t nr_size;

#ifdef NR_EXPORT

#ifdef _MSC_VER
	#define NRAPI __declspec(dllexport)
#else
	#define NRAPI
#endif // _MSC_VER

#else

#ifdef _MSC_VER
	#define NRAPI __declspec(dllimport)
#else
	#define NRAPI
#endif // _MSC_VER


#endif // NR_EXPORT

namespace nr
{

typedef std::basic_string<nr_char> string;

namespace error
{
	NRAPI nr_bool isSuccess(const cl_status& err);
	NRAPI nr_bool isFailure(const cl_status& err);
}

}

