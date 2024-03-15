#ifndef SIGLOT_CONNECTION_H
#define SIGLOT_CONNECTION_H

#include <iostream>
#include <memory>

namespace siglot
{

class Object;

namespace Connection
{

enum ConnectionType
{
    AUTO,
    QUEUED,
    DIRECT,
};

struct GeneralizedConnection
{
    GeneralizedConnection(
            Object *signalObject, const std::string &signalName,
            Object *slotObject, const std::string &slotName,
            ConnectionType connectionType,
            bool isHiddenInGraphViz);
    virtual ~GeneralizedConnection() = default;
    Object *mSignalObject, *mSlotObject;
    const std::string mSignalName, mSlotName, mSignalId, mSlotId;
    const ConnectionType mConnectionType;
    const bool mIsHiddenInGraphViz;

    // these are atomic since they are accessed in a shared-locked section in EObject::emit()
    std::atomic<size_t> mCallCount;
    std::atomic<float> mCallFrequency;
    std::atomic<long long> mLastCallTime;
};

template<typename ... ArgTypes>
struct Connection : public GeneralizedConnection
{
    // with param
    template<typename SignalObjectType, typename SignalObjectBaseType, typename SlotObjectType, typename SlotObjectBaseType>
    Connection(
            SignalObjectType *signalObject,
            const std::string &signalName,
            void (SignalObjectBaseType::*signal)(ArgTypes &&...),
            SlotObjectType *slotObject,
            const std::string &slotName,
            void (SlotObjectBaseType::*slot)(ArgTypes &&...),
            ConnectionType connectionType, bool isHiddenInGraphViz)
            : GeneralizedConnection(signalObject, signalName, slotObject, slotName, connectionType, isHiddenInGraphViz)
    {
        static_assert(std::is_convertible<SignalObjectType *, SignalObjectBaseType *>::value,
                      "Derived must inherit Base as public");
        static_assert(std::is_convertible<SlotObjectType *, SlotObjectBaseType *>::value,
                      "Derived must inherit Base as public");

        mSignalObject = signalObject;
        mSlotObject = slotObject;
        mSlotCaller = [=](ArgTypes &&... args)
        {
            if constexpr (sizeof...(ArgTypes) == 0U)
                (slotObject->*slot)();
            else
                (slotObject->*slot)(std::move(args)...);
        };
    }

    std::function<void(ArgTypes && ...)> mSlotCaller;
};

}
}
#endif
