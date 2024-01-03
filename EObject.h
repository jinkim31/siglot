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
    : GeneralizedConnection(std::type_index(typeid(signal)), std::type_index(typeid(slot)))
    {
        mSignalObject = signalObject;
        mSlotObject = slotObject;
        mSlotCaller = [=](ArgTypes... args){ (slotObject->*slot)(args...); }; // copy capture since slotObject will go out of scope
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
            // find connection
            if(!(connection->mSignalObject == this && connection->mSignalId == std::type_index(typeid(signal))))
                continue;

            // cast connection to typed connection
            // TODO: use static_cast instead after sufficient testing
            auto *argTypeConnection = dynamic_cast<Connection<ArgTypes...> *>(connection.get());
            if (argTypeConnection == nullptr)
            {
                std::cerr << "cast failed" << std::endl;
                return;
            }

            // find signal slot thread
            auto signalThread = ELookup::instance().mObjectThreadMap.find(connection->mSignalObject) ;
            auto slotThread = ELookup::instance().mObjectThreadMap.find(connection->mSlotObject) ;

            // check thread validity
            if(signalThread == ELookup::instance().mObjectThreadMap.end())
            {
                std::cerr<<"Signal EObject is not in any thread. use EObject::move(EThread*) to assign it to a thread."<<std::endl;
                return;
            }
            if(slotThread == ELookup::instance().mObjectThreadMap.end())
            {
                std::cerr<<"Slot EObject is not in any thread. use EObject::move(EThread*) to assign it to a thread."<<std::endl;
                return;
            }

            // execute if signal and slot objects are in the same thread
            if( ELookup::instance().mObjectThreadMap[this] == ELookup::instance().mObjectThreadMap[connection->mSlotObject])
            {
                argTypeConnection->mSlotCaller(args...);
            }
            // queue if signal and slot objects are in different threads
            else
            {
                ELookup::instance().mObjectThreadMap[connection->mSlotObject]->pushEvent(
                        std::bind(argTypeConnection->mSlotCaller, args...));
            }
        }
    }
};


#endif
