#include <base/Context.h>

namespace nr
{

void Context::makeDefault(const Context& provided)
{
    defaultContext = provided;
}

Context Context::getDefault()
{
    return defaultContext;
}

Context::Context()
    : Wrapped()
{
}

Context::Context(const cl_context& context, const nr_bool retain)
    : Wrapped(context, retain)
{
}

Context::Context(const Context& other)
    : Wrapped(other)
{
}

Context::Context(Context&& other)
    : Wrapped(other)
{
}

Context::Context(
    const cl_context_properties* properties, 
    std::vector<Device>& devices, 
    cl_status* err)
    : Wrapped(
        clCreateContext(
            properties, 
            devices.size(), &devices.front().get(), 
            nullptr, nullptr, 
            err))
{
}

Context::Context(
    const cl_context_properties* properties, 
    cl_device_type deviceType,          
    cl_status* err)
    : Wrapped(
        clCreateContextFromType(
            properties, 
            deviceType,
            nullptr, nullptr, 
            err))
{
}

Context& Context::operator=(const Context& other)
{
    Wrapped::operator=(other);
    return *this;
}

Context& Context::operator=(Context&& other)
{
    Wrapped::operator=(other);
    return *this;
}

Context::operator cl_context() const 
{
    return object;
}

Context Context::defaultContext = Context();

}