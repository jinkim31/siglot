//
// Created by Jin Kim on 2024/01/02.
//

#ifndef ETHREAD2_EOBJECT_H
#define ETHREAD2_EOBJECT_H

#include <vector>
#include <map>
#include <iostream>
#include <functional>
#include <memory>
#include "EThread.h"
#include "ELookup.h"

class EObject;
class EThread;

template <typename... ArgTypes>
struct Connection : public GeneralizedConnection
{
    template <typename SignalObjectType, typename SlotObjectType>
    Connection(
            SignalObjectType* signalObject,
            void (SignalObjectType::*signal)(ArgTypes...),
            SlotObjectType* slotObject,
            void (SlotObjectType::*slot)(ArgTypes...))
    {
        //mSignalHash = reinterpret_cast<intptr_t>(&signal);
        mSignalObject = signalObject;
        mSlotObject = slotObject;
        mSignalHash = typeid(signal).hash_code();
        mSlotHash = typeid(slot).hash_code();
        std::cout<<"connecting signal "<<mSignalHash<<" to slot "<<mSlotHash<<std::endl;
        mSignalObject = signalObject;
        mSlotObject = slotObject;
        mSlotCaller = std::bind(slot, slotObject, std::placeholders::_1);
    }
    std::function<void(ArgTypes...)> mSlotCaller;
};


class EObject
{
public:
    template <typename SignalObjectType, typename SlotObjectType, typename... ArgTypes>
    static void connect(
            SignalObjectType* signalObject, void (SignalObjectType::*signal)(ArgTypes...),
            SlotObjectType* slotObject, void (SlotObjectType::*slot)(ArgTypes...))
    {
        ELookup::instance().addConnection(
                std::unique_ptr<Connection<ArgTypes...>>(new Connection(signalObject, signal, slotObject, slot)));
    }

    void move(EThread& ethread);

    void remove();

    template <typename SignalObjectType, typename... ArgTypes>
    void emit(void (SignalObjectType::*signal)(ArgTypes...), ArgTypes... args)
    {
        // shared-lock lookup
        std::shared_lock<std::shared_mutex> lock(ELookup::instance().getMutex());

        // find connection
        // TODO: optimize search
        for(auto& connection  : ELookup::instance().mConnectionGraph)
        {
            if(connection->mSignalObject == this && connection->mSignalHash == typeid(signal).hash_code())
            {
                // std::cout<<"found slot"<<std::endl;
                auto* argTypeConnection = dynamic_cast<Connection<ArgTypes...>*>(connection.get());
                if(argTypeConnection == nullptr)
                {
                    std::cerr<<"cast failed"<<std::endl;
                    return;
                }
                ELookup::instance().mObjectThreadMap[connection->mSlotObject]->pushEvent(
                        std::bind(argTypeConnection->mSlotCaller, args...));
            }
        }
    }
};


#endif
