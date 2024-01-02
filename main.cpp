#include <iostream>
#include "EObject.h"

class SignalEObject : public EObject
{
public:
    void mySignal(std::string message){}
    void emitSignal()
    {
        EObject::emit<std::string>("hello world");
    }
};

class SlotEObject : public EObject
{
public:
    void mySlot(std::string message)
    {
        std::cout<<message<<std::endl;
    }
};

int main()
{
    SignalEObject signalEObject;
    SlotEObject slotEObject;

    // connect
    EObject::connect(&signalEObject, &SignalEObject::mySignal, &slotEObject, &SlotEObject::mySlot);
    // emit signal
    signalEObject.emitSignal();
}
