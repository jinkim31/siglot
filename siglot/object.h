#ifndef SIGLOT_OBJECT_H
#define SIGLOT_OBJECT_H

#define SIGNAL void
#define SLOT void

#define SIGLOT(x) #x, & x

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
            SignalObjectType &signalObject, const std::string &signalId, void (SignalObjectBaseType::*signal)(ArgTypes...),
            SlotObjectType &slotObject, const std::string &slotId, void (SlotObjectBaseType::*slot)(ArgTypes...),
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

    template<typename SignalObjectType, typename... ArgTypes>
    void emit(const std::string &signalName, void (SignalObjectType::*signal)(ArgTypes...), ArgTypes&&... args)
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

            // connect with given connection type
            switch (connection->mConnectionType)
            {
                case Connection::AUTO:
                {
                    if (sameThread)
                        argTypeConnection->mSlotCaller(args...);
                    else
                        slotThread->pushEvent(argTypeConnection->mSlotObject,
                                              std::bind(argTypeConnection->mSlotCaller, args...));
                    break;
                }
                case Connection::DIRECT:
                {
                    if (!sameThread)
                        throw std::runtime_error("Direct connection between EObject from different threads.");
                    argTypeConnection->mSlotCaller(args...);
                    break;
                }
                case Connection::QUEUED:
                {
                    slotThread->pushEvent(argTypeConnection->mSlotObject, std::bind(argTypeConnection->mSlotCaller, args...));
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
