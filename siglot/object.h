#ifndef SIGLOT_OBJECT_H
#define SIGLOT_OBJECT_H

#define SIGNAL void
#define SLOT void
#define SIGLOT(x) #x, & x

#define SIGLOT_ADD_FROM_FUNC(retType, className, funcName, param1Type, param1Name)\
void SIGNAL_##funcName ( retType && ){};\
void  SLOT_##funcName ( param1Type && param1Name)\
{ emit( #className "::SIGNAL_" #funcName, & className :: SIGNAL_##funcName , std::move(funcName(std::move(param1Name)))); }

#define SIGLOT_ADD_FROM_FUNC_2(retType, className, funcName, param1Type, param1Name, param2Type, param2Name)\
void SIGNAL_##funcName ( retType && ){};\
void  SLOT_##funcName ( param1Type && param1Name , param2Type && param2Name )\
{ emit( #className "::SIGNAL_" #funcName, & className :: SIGNAL_##funcName , std::move(funcName(std::move(param1Name), std::move(param2Name)))); }

#define SIGLOT_ADD_FROM_FUNC_3(retType, className, funcName, param1Type, param1Name, param2Type, param2Name, param3Type, param3Name)\
void SIGNAL_##funcName ( retType && ){};\
void  SLOT_##funcName ( param1Type && param1Name , param2Type && param2Name , param3Type && param3Name)\
{ emit( #className "::SIGNAL_" #funcName, & className :: SIGNAL_##funcName , std::move(funcName(std::move(param1Name), std::move(param2Name), std::move(param3Name)))); }


#include <vector>
#include <map>
#include <iostream>
#include <functional>
#include <memory>
#include "thread.h"
#include "lookup.h"
#include "connection.h"
#include "util.h"

namespace siglot
{
class Object;

class Thread;

class Object
{
public:
    Object();

    template<typename SignalObjectType, typename SignalObjectBaseType, typename SlotObjectType, typename SlotObjectBaseType, typename... ArgTypes>
    static void connect(
            SignalObjectType &signalObject, const std::string &signalId, void (SignalObjectBaseType::*signal)(ArgTypes&&...),
            SlotObjectType &slotObject, const std::string &slotId, void (SlotObjectBaseType::*slot)(ArgTypes&&...),
            Connection::ConnectionType connectionType = Connection::AUTO,
            bool isHiddenInGraphViz = false)
    {
        // check signal slot inheritance
        static_assert(std::is_convertible<SignalObjectType*, SignalObjectBaseType*>::value, "Derived must inherit Base as public");
        static_assert(std::is_convertible<SlotObjectType*, SlotObjectBaseType*>::value, "Derived must inherit Base as public");

        //std::cout<<"connecting "<<signalObject<<"-"<<signalId<<" to "<<slotObject<<"-"<<slotId<<std::endl;
        std::unique_lock<std::shared_mutex> lock(Lookup::instance().getGlobalMutex());
        Lookup::instance().unprotectedAddConnection(
                std::unique_ptr<Connection::Connection<ArgTypes...>>(
                        new Connection::Connection(&signalObject, signalId, signal,
                                                   &slotObject, slotId, slot,
                                                   connectionType, isHiddenInGraphViz)));
    }

    void move(Thread &ethread);

    void remove();

    //move. first parameter signalName is non-const to resolve overload ambiguity when ArgType is void
    template<typename SignalObjectType, typename... ArgTypes>
    void emit(std::string &signalName, void (SignalObjectType::*signal)(ArgTypes&&...), ArgTypes&&... args)
    {
        // shared-lock lookup
        std::shared_lock<std::shared_mutex> lock(Lookup::instance().getGlobalMutex());

        // find connection
        // TODO: optimize search
        //std::cout<<"target"<<this<<"::"<<std::type_index(typeid(signal)).name()<<std::endl;
        for (auto &connection: Lookup::instance().mConnectionGraph)
        {
            // find connection
            if (!(connection->mSignalObject == this && connection->mSignalId == functionNameWithNamespaceToSiglotId(signalName)))
                continue;

            // update connection call frequency
            auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();
            long long timeNow = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
            auto currentCallFrequency = 1000000.0f / (timeNow - connection->mLastCallTime);
            if(connection->mCallCount==0)
            {
                connection->mCallFrequency = currentCallFrequency;
            }
            else
            {
                connection->mCallFrequency = (connection->mCallCount * connection->mCallFrequency + currentCallFrequency) /
                                             (connection->mCallCount + 1);
                connection->mLastCallTime = timeNow;
            }

            // increment connection call counter
            connection->mCallCount++;

            // cast connection to typed connection
            // TODO: use static_cast instead after sufficient testing. Might be better to continue using dynamic_cast to support inherited signal, slot
            auto *argTypeConnection = dynamic_cast<Connection::Connection<ArgTypes...> *>(connection.get());
            if (argTypeConnection == nullptr)
            {
                std::cerr << "cast failed" << std::endl;
                return;
            }

            // find signal slot thread
            auto signalThread = argTypeConnection->mSignalObject->mThreadInAffinity;
            auto slotThread = argTypeConnection->mSlotObject->mThreadInAffinity;

            // check thread validity
            if (!signalThread)
            {
                std::cerr
                        << "Signal EObject is not in any thread. use EObject::move(EThread*) to assign it to a thread."
                        << std::endl;
                return;
            }
            if (!slotThread)
            {
                std::cerr << "Slot EObject is not in any thread. use EObject::move(EThread*) to assign it to a thread."
                          << std::endl;
                return;
            }

            // check if signal and slot objects are in the same thread
            bool sameThread = signalThread == slotThread;

            // make event
            std::function<void()> event;
            if constexpr (sizeof...(ArgTypes) == 0U)
                event = argTypeConnection->mSlotCaller;
            else
                event = [=, ... args = std::move(args)]()mutable{
                    (argTypeConnection->mSlotCaller)(std::move(args)...);
                };

            // connect with given connection type
            switch (connection->mConnectionType)
            {
                case Connection::AUTO:
                {
                    if (sameThread)
                        event();
                    else
                        slotThread->pushEvent(argTypeConnection->mSlotObject,std::move(event));
                    break;
                }
                case Connection::DIRECT:
                {
                    if (!sameThread)
                        throw std::runtime_error("Direct connection between EObject from different threads.");
                    event();
                    break;
                }
                case Connection::QUEUED:
                {
                    slotThread->pushEvent(argTypeConnection->mSlotObject, std::move(event));
                    break;
                }
            }
        }
    }


    //copy. first parameter signalName is const to resolve overload ambiguity when ArgType is void
    template<typename SignalObjectType, typename... ArgTypes>
    void emit(const std::string &signalName, void (SignalObjectType::*signal)(ArgTypes&&...), ArgTypes... args)
    {
        // shared-lock lookup
        std::shared_lock<std::shared_mutex> lock(Lookup::instance().getGlobalMutex());

        // find connection
        // TODO: optimize search
        //std::cout<<"target"<<this<<"::"<<std::type_index(typeid(signal)).name()<<std::endl;
        for (auto &connection: Lookup::instance().mConnectionGraph)
        {
            // find connection
            if (!(connection->mSignalObject == this && connection->mSignalId == functionNameWithNamespaceToSiglotId(signalName)))
                continue;

            // update connection call frequency
            auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();
            long long timeNow = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
            auto currentCallFrequency = 1000000.0f / (timeNow - connection->mLastCallTime);
            if(connection->mCallCount==0)
            {
                connection->mCallFrequency = currentCallFrequency;
            }
            else
            {
                connection->mCallFrequency = (connection->mCallCount * connection->mCallFrequency + currentCallFrequency) /
                                             (connection->mCallCount + 1);
                connection->mLastCallTime = timeNow;
            }

            // increment connection call counter
            connection->mCallCount++;

            // cast connection to typed connection
            // TODO: use static_cast instead after sufficient testing. Might be better to continue using dynamic_cast to support inherited signal, slot
            auto *argTypeConnection = dynamic_cast<Connection::Connection<ArgTypes...> *>(connection.get());
            if (argTypeConnection == nullptr)
            {
                std::cerr << "cast failed" << std::endl;
                return;
            }

            // find signal slot thread
            auto signalThread = argTypeConnection->mSignalObject->mThreadInAffinity;
            auto slotThread = argTypeConnection->mSlotObject->mThreadInAffinity;

            // check thread validity
            if (!signalThread)
            {
                std::cerr
                        << "Signal EObject is not in any thread. use EObject::move(EThread*) to assign it to a thread."
                        << std::endl;
                return;
            }
            if (!slotThread)
            {
                std::cerr << "Slot EObject is not in any thread. use EObject::move(EThread*) to assign it to a thread."
                          << std::endl;
                return;
            }

            // check if signal and slot objects are in the same thread
            bool sameThread = signalThread == slotThread;

            // make event
            std::function<void()> event;
            if constexpr (sizeof...(ArgTypes) == 0U)
                event = argTypeConnection->mSlotCaller;
            else
                event = [=, ... args = std::move(args)]()mutable{
                    (argTypeConnection->mSlotCaller)(std::move(args)...);
                };

            // connect with given connection type
            switch (connection->mConnectionType)
            {
                case Connection::AUTO:
                {
                    if (sameThread)
                        event();
                    else
                        slotThread->pushEvent(argTypeConnection->mSlotObject,std::move(event));
                    break;
                }
                case Connection::DIRECT:
                {
                    if (!sameThread)
                        throw std::runtime_error("Direct connection between EObject from different threads.");
                    event();
                    break;
                }
                case Connection::QUEUED:
                {
                    slotThread->pushEvent(argTypeConnection->mSlotObject, std::move(event));
                    break;
                }
            }
        }
    }

    template<typename SlotObjectType, typename SlotObjectBaseType, typename... ArgTypes>
    static void callSlot(SlotObjectType& slotObject,
                  const std::string &slotName, void (SlotObjectBaseType::*slot)(ArgTypes...), ArgTypes... args)
    {
        // slot object and slot type check
        static_assert(std::is_convertible<SlotObjectType*, SlotObjectBaseType*>::value, "Derived must inherit Base as public");

        // shared-lock lookup
        std::shared_lock<std::shared_mutex> lock(Lookup::instance().getGlobalMutex());
        auto signalThreadId = std::this_thread::get_id();
        auto slotThreadId = slotObject.mThreadInAffinity->mThread.get_id();

        // check if signal and slot objects are in the same thread
        bool sameThread = signalThreadId == slotThreadId;

        // connect with given connection type
        if (sameThread)
        {
            (slotObject.*slot)(args...);
        }
        else
        {
            slotObject.mThreadInAffinity->pushEvent(&slotObject, std::move(std::bind(slot, &slotObject, args...)));
        }
    }

    void setName(const std::string &name);
    std::string name();
protected:
    virtual void onMove(Thread &thread)
    {}

    virtual void onRemove()
    {}

private:
    Thread *mThreadInAffinity;
    std::string mName;
    friend Thread;
    friend Lookup;
};

}
#endif
