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
    const std::string  mSignalName, mSlotName, mSignalId, mSlotId;
    const ConnectionType mConnectionType;
    const bool mIsHiddenInGraphViz;

    // these are atomic since they are accessed in a shared-locked section in EObject::emit()
    std::atomic<size_t> mCallCount;
    std::atomic<float> mCallFrequency;
    std::atomic<long long> mLastCallTime;
};

template<typename... ArgTypes>
struct Connection : public GeneralizedConnection
{
    template<typename SignalObjectType, typename SlotObjectType>
    Connection(
            SignalObjectType *signalObject,
            const std::string &signalName,
            void (SignalObjectType::*signal)(ArgTypes...),
            SlotObjectType *slotObject,
            const std::string &slotName,
            void (SlotObjectType::*slot)(ArgTypes...),
            ConnectionType connectionType, bool isHiddenInGraphViz)
            : GeneralizedConnection(signalObject, signalName, slotObject, slotNamefixe, connectionType, isHiddenInGraphViz)
    {
        mSignalObject = signalObject;
        mSlotObject = slotObject;
        mSlotCaller = [=](ArgTypes... args)
        { (slotObject->*slot)(args...); }; // copy capture since slotObject will go out of scope
    }

    std::function<void(ArgTypes...)> mSlotCaller;
};

}
}
#endif
