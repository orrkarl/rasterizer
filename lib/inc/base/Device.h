#pragma once

#include "../general/predefs.h"

#include "Wrapper.h"

namespace nr
{

class NR_SHARED Device : public Wrapper<cl_device_id>
{
// Functions and Constructors
public:
    static makeDefault(Device provided);

    static getDefault();

    Device();

    explicit Device(const cl_device_id& Device, const NRbool retain = false);

    Device(const Device& other);

    Device(Device&& other);

    Device& operator=(const Device& other);

    Device& operator=(Device&& other);

    operator cl_device_id() const;

// Fields
private:
    static Device defaultDevice;
};

}