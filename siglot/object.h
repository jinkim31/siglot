#ifndef SIGLOT_OBJECT_H
#define SIGLOT_OBJECT_H

#define SIGNAL
#define SLOT

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

    template<typename SignalObjectType, typename SlotObjectType, typename... ArgTypes>
    static void connect(
            SignalObjectType &signalObject, const std::string &signalId, void (SignalObjectType::*signal)(ArgTypes...),
            SlotObjectType &slotObject, const std::string &slotId, void (SlotObjectType::*slot)(ArgTypes...),
            Connection::ConnectionType connectionType = Connection::AUTO,
            bool isHiddenInGraphViz = false)
    {
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
    void emit(const std::string &signalName, void (SignalObjectType::*signal)(ArgTypes...), ArgTypes... args)
    {
        // shared-lock lookup
        std::shared_lock<std::shared_mutex> lock(Lookup::instance().getGlobalMutex());

        // find connection
        // TODO: optimize search
        //std::cout<<"target"<<this<<"::"<<std::type_index(typeid(signal)).name()<<std::endl;
        for (auto &connection: Lookup::instance().mConnectionGraph)
        {
            // find connection
            //std::cout<<"iter"<<connection->mSignalObject<<"-"<<connection->mSignalId<<std::endl;
            if (!(connection->mSignalObject == this && connection->mSignalId == functionNameWithNamespaceToSiglotId(signalName)))
                continue;

            // cast connection to typed connection
            // TODO: use static_cast instead after sufficient testing. Might be better to continue using dynamic_cast to support inherited signal, slot
            auto *argTypeConnection = dynamic_cast<Connection::Connection<ArgTypes...> *>(connection.get());
            if (argTypeConnection == nullptr)
            {
                std::cerr << "cast failed" << std::endl;
                return;
            }

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

            // find signal slot thread
            auto signalThread = connection->mSignalObject->mThreadInAffinity;
            auto slotThread = connection->mSlotObject->mThreadInAffinity;

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
            switch (argTypeConnection->mConnectionType)
            {
                case Connection::AUTO:
                {
                    if (sameThread)
                        argTypeConnection->mSlotCaller(args...);
                    else
                        slotThread->pushEvent(connection->mSlotObject,
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
                    slotThread->pushEvent(connection->mSlotObject, std::bind(argTypeConnection->mSlotCaller, args...));
                    break;
                }
            }
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
