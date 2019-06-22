#pragma once

#include "../general/predefs.h"
#include "Wrapper.h"

namespace nr
{

/**
 * A wrapper for kernel
 **/
class NR_SHARED Kernel : public Wrapper<cl_kernel>
{
public:
    Kernel();

    explicit Kernel(const cl_kernel& Kernel, const NRbool retain = false);

    Kernel(const Kernel& other);

    Kernel(Kernel&& other);

    Kernel& operator=(const Kernel& other);

    Kernel& operator=(Kernel&& other);

    operator cl_kernel() const;

    template<typename T>
    typename std::enable_if<!std::is_pointer<T>() && std::is_pod<T>(), cl_status>::type setArg(const NRuint index, const T& value)
    {
        return clSetKernelArg(object, index, sizeof(T), value);
    }
    
    template<typename T>
    cl_status setArg(const NRuint index, const NRuint size, const T value)
    {
        return clSetKernelArg(object, index, size, value);
    }
};

}
