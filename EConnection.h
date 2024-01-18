#ifndef ETHREAD2_ECONNECTION_H
#define ETHREAD2_ECONNECTION_H

#include <iostream>
#include <memory>
#include <typeindex>

class EObject;

class EConnection
{
public:
    enum ConnectionType{
        AUTO,
        QUEUED,
        DIRECT,
    };
    struct GeneralizedConnection
    {
        GeneralizedConnection(
                EObject *signalObject, std::type_index signalId,
                EObject *slotObject, std::type_index slotId,
                ConnectionType connectionType);
        virtual ~GeneralizedConnection() = default;
        EObject *mSignalObject, *mSlotObject;
        const std::type_index mSignalId, mSlotId;
        const ConnectionType mConnectionType;
    };

    template<typename... ArgTypes>
    struct Connection : public GeneralizedConnection
    {
        template<typename SignalObjectType, typename SlotObjectType>
        Connection(
                SignalObjectType *signalObject,
                void (SignalObjectType::*signal)(ArgTypes...),
                SlotObjectType *slotObject,
                void (SlotObjectType::*slot)(ArgTypes...),
                ConnectionType connectionType)
                : GeneralizedConnection(signalObject, std::type_index(typeid(signal)), slotObject, std::type_index(typeid(slot)), connectionType)
        {
            mSignalObject = signalObject;
            mSlotObject = slotObject;
            mSlotCaller = [=](ArgTypes... args){ (slotObject->*slot)(args...); }; // copy capture since slotObject will go out of scope
        }

        std::function<void(ArgTypes...)> mSlotCaller;
    };

};
#endif //ETHREAD2_ECONNECTION_H
