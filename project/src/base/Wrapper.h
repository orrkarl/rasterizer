/**
 * @file Wrapper.h
 * @author Orr Karl (karlor041@gmail.com)
 * @brief Wrapping OpenCL built in reference handling, implementing copy and move constructors and operators accordingly
 * @see ReferenceHandler.h
 * @version 0.5.9
 * @date 2019-06-30
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#pragma once

#include "../predefs.h"

#include "ReferenceHandler.h"

namespace nr
{

/**
 * @brief Wrapping class for basic OpenCL types, allowing to copy and move them while updating their referene count correctly 
 * 
 */
template<typename cl_type>
class Wrapper
{
public:
    Wrapper() 
        : object(nullptr)
    {
    }

    explicit Wrapper(const cl_type object, const nr_bool retainObject = false)
        : object(object)
    {
        if (retainObject) retain();
    }

    ~Wrapper()
    {
        release();
    }

    Wrapper(const Wrapper<cl_type>& other)
        : object(other.object)
    {
        retain();
    }

    Wrapper(Wrapper<cl_type>&& other)
    {
        object = other.object;
        other.object = nullptr;
    }

    Wrapper<cl_type>& operator=(const Wrapper<cl_type>& other)
    {
        if (this != &other)
        {
            release();
            object = other.object;
            retain();
        }

        return *this;
    }

    Wrapper<cl_type>& operator=(Wrapper<cl_type>&& other)
    {
        if (this != &other)
        {
            release();
            object = other.object;
            other.object = nullptr;
        }

        return *this;
    }

    cl_status release() 
    { 
        cl_status ret = object != nullptr ? ReferenceHandler<cl_type>::release(object) : CL_SUCCESS; 
        object = nullptr;
        return ret;
    }

    cl_status retain() 
    { 
        return object != nullptr ? ReferenceHandler<cl_type>::retain(object) : CL_SUCCESS; 
    }

protected:
    typedef Wrapper<cl_type> Wrapped;

    cl_type object;
};

}