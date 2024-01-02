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

class EObject;

struct GeneralizedConnection{
    virtual ~GeneralizedConnection() = default;
};
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
        mSignalHash = reinterpret_cast<intptr_t>(&signal);
        mSlotHash = reinterpret_cast<intptr_t>(&slot);
        std::cout<<"connecting signal "<<mSignalHash<<" to slot "<<mSlotHash<<std::endl;
        mSignalObject = signalObject;
        mSlotObject = slotObject;
        mSlotCaller = std::bind(slot, slotObject, std::placeholders::_1);
    }
    EObject* mSignalObject, *mSlotObject;
    intptr_t mSignalHash, mSlotHash;
    std::function<void(ArgTypes...)> mSlotCaller;
};


class EObject
{
public:
    static std::vector<std::unique_ptr<GeneralizedConnection>> mConnectionGraph;

public:
    template <typename SignalObjectType, typename SlotObjectType, typename... ArgTypes>
    static void connect(
            SignalObjectType* signalObject,
            void (SignalObjectType::*signal)(ArgTypes...),
            SlotObjectType* slotObject,
            void (SlotObjectType::*slot)(ArgTypes...))
    {
        mConnectionGraph.emplace_back(
                std::unique_ptr<Connection<ArgTypes...>>(new Connection(signalObject, signal, slotObject, slot)));
    }

    template <typename... ArgTypes>
    static void emit(ArgTypes... args)
    {
        // find connection
        auto connection= EObject::mConnectionGraph[0].get();

        // cast connection
        auto* argTypeConnection = dynamic_cast<Connection<std::string>*>(connection); // might be ok to use static_cast
        if(argTypeConnection == nullptr)
        {
            std::cerr<<"cast failed"<<std::endl;
            return;
        }
        argTypeConnection->mSlotCaller(args...);
    }
};


#endif
