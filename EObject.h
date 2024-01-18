#ifndef ETHREAD2_EOBJECT_H
#define ETHREAD2_EOBJECT_H

#define SIGNAL
#define SLOT

#include <vector>
#include <map>
#include <iostream>
#include <functional>
#include <memory>
#include "EThread.h"
#include "ELookup.h"
#include "EConnection.h"

class EObject;
class EThread;

class EObject
{
public:
    template <typename SignalObjectType, typename SlotObjectType, typename... ArgTypes>
    static void connect(
            SignalObjectType* signalObject, void (SignalObjectType::*signal)(ArgTypes...),
            SlotObjectType* slotObject, void (SlotObjectType::*slot)(ArgTypes...),
            EConnection::ConnectionType connectionType = EConnection::AUTO)
    {
        std::cout<<"connecting "<<signalObject<<"::"<<std::type_index(typeid(signal)).hash_code()<<" to "<<slotObject<<"::"<<std::type_index(typeid(slot)).name()<<std::endl;
        std::unique_lock<std::shared_mutex> lock(ELookup::instance().getMutex());
        ELookup::instance().addConnection(
                std::unique_ptr<EConnection::Connection<ArgTypes...>>(
                        new EConnection::Connection(signalObject, signal, slotObject, slot, connectionType)));
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
        std::cout<<"target"<<this<<"::"<<std::type_index(typeid(signal)).name()<<std::endl;
        for(auto& connection  : ELookup::instance().mConnectionGraph)
        {
            // find connection
            std::cout<<"iter"<<connection->mSignalObject<<"::"<<connection->mSignalId.name()<<std::endl;
            if(!(connection->mSignalObject == this && connection->mSignalId == std::type_index(typeid(signal))))
                continue;

            // cast connection to typed connection
            // TODO: use static_cast instead after sufficient testing. Might be better to continue using dynamic_cast to support inherited signal, slot
            auto *argTypeConnection = dynamic_cast<EConnection::Connection<ArgTypes...> *>(connection.get());
            if (argTypeConnection == nullptr)
            {
                std::cerr << "cast failed" << std::endl;
                return;
            }

            // find signal slot thread
            auto signalThread = ELookup::instance().mObjectThreadMap.find(connection->mSignalObject);
            auto slotThread = ELookup::instance().mObjectThreadMap.find(connection->mSlotObject);

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

            // check if signal and slot objects are in the same thread
            bool sameThread = ELookup::instance().mObjectThreadMap[this] == ELookup::instance().mObjectThreadMap[connection->mSlotObject];

            switch(argTypeConnection->mConnectionType)
            {
            case EConnection::AUTO:
            {
                if(sameThread)
                    argTypeConnection->mSlotCaller(args...);
                else
                    ELookup::instance().mObjectThreadMap[connection->mSlotObject]
                    ->pushEvent(this,std::bind(argTypeConnection->mSlotCaller,args...));
                break;
            }
            case EConnection::DIRECT:
            {
                if (!sameThread)
                    throw std::runtime_error("Direct connection between EObject from different threads.");
                argTypeConnection->mSlotCaller(args...);
                break;
            }
            case EConnection::QUEUED:
            {
                ELookup::instance().mObjectThreadMap[connection->mSlotObject]
                ->pushEvent(this, std::bind(argTypeConnection->mSlotCaller, args...));
                break;
            }
            }
        }
    }
protected:
    virtual void onMove(EThread& thread){}
    virtual void onRemove(){}
};


#endif
