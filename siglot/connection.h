#ifndef ETHREAD2_ECONNECTION_H
#define ETHREAD2_ECONNECTION_H

#include <iostream>
#include <memory>
#include <typeindex>

class Object;

namespace Connection
{

enum ConnectionType{
    AUTO,
    QUEUED,
    DIRECT,
};
struct GeneralizedConnection
{
    GeneralizedConnection(
            Object *signalObject, const std::string& signalId,
            Object *slotObject, const std::string& slotId,
            ConnectionType connectionType,
            bool isHiddenInGraphViz);
    virtual ~GeneralizedConnection() = default;
    Object *mSignalObject, *mSlotObject;
    const std::string mSignalId, mSlotId;
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
            const std::string& signalId,
            void (SignalObjectType::*signal)(ArgTypes...),
            SlotObjectType *slotObject,
            const std::string& slotId,
            void (SlotObjectType::*slot)(ArgTypes...),
            ConnectionType connectionType, bool isHiddenInGraphViz)
            : GeneralizedConnection(signalObject, signalId, slotObject, slotId, connectionType, isHiddenInGraphViz)
    {
        mSignalObject = signalObject;
        mSlotObject = slotObject;
        mSlotCaller = [=](ArgTypes... args){ (slotObject->*slot)(args...); }; // copy capture since slotObject will go out of scope
    }

    std::function<void(ArgTypes...)> mSlotCaller;
};

}
#endif //ETHREAD2_ECONNECTION_H
