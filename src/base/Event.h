#pragma once

#include "../predefs.h"

#include <functional>
#include <variant>
#include <vector>

#include "Context.h"
#include "Wrapper.h"

namespace nr::base {

CL_TYPE_CREATE_EXCEPTION(Event, CLApiException);
CL_TYPE_CREATE_EXCEPTION(ApiEvent, EventCreateException);

class EventWaitException : public CLApiException {
public:
    explicit EventWaitException(Status status);
};

class BadEventExecutionStatus : public CLApiException {
public:
    explicit BadEventExecutionStatus(Status status);
};

class Event {
public:
    enum class ExecutionStatus : cl_int {
        Queued = CL_QUEUED,
        Submitted = CL_SUBMITTED,
        Running = CL_RUNNING,
        Completed = CL_COMPLETE
    };

    /**
     * @brief Blocks until all events in list reached CL_COMPLETE
     *
     * @note although it accepts a vector, passing more than MAX_UINT elements will throw
     *
     * @param events event list to wait
     * @return cl_status internal OpenCL call status
     */
    static void await(const std::vector<Event>& events);

    /**
     * @brief Blocks until this event reaches CL_COMPLETE
     * @note wraps clWaitForEvents
     * @return cl_status internal OpenCL call status
     */
    void await() const;

    /**
     * @brief Queries the status of the operation this event waits on, throws if the event-related operation
     * failed
     *
     * @throws BadEventExecutionStatus if the operation this event notifies failed
     * @return event execution status
     */
    ExecutionStatus status() const;

protected:
    explicit Event(cl_event rawEvent);

private:
    friend class CommandQueue;

    struct EventTraits {
        using Type = cl_event;

        static constexpr auto release = clReleaseEvent;
        static constexpr auto retain = clRetainEvent;
    };

    cl_event rawHandle() const;

    UniqueWrapper<EventTraits> m_object;
};

class UserEvent : public Event {
public:
    explicit UserEvent(Context& context);
};

class ApiEvent : public Event {
private:
    friend class CommandQueue;

    explicit ApiEvent(cl_event rawEvent);
};

}
