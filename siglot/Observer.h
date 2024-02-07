#ifndef ETHREAD2_EOBSERVER_H
#define ETHREAD2_EOBSERVER_H

#include "Object.h"
#include "Lookup.h"
#include <chrono>

class Observer : public Object
{
public:
    Observer(bool emitObservedSignal = true)
    {
        mIsActive = false;
        mEmitObservedSignal = emitObservedSignal;
        setName("observer");
        //TODO: this line outputs error: EXC_BAD_ACCESS connect(this, &EObserver::selfCallSignal, this, &EObserver::selfCallSlot, EConnection::DIRECT);
        connect(this, SIGLOT(Observer::selfCallSignal), this, SIGLOT(Observer::selfCallSlot), Connection::QUEUED, true);
    }
    void start()
    {
        if(mIsActive)
            return;
        mIsActive = true;
        emit(SIGLOT(Observer::selfCallSignal));
    }
    void stop()
    {
        if(!mIsActive)
            return;
        mIsActive = false;
    }

    void SIGNAL observed(){std::cout<<"a";}
protected:
    virtual void observerCallback()
    {
        //std::cout<<"EObserver callback"<<std::endl;
    }
private:
    void SIGNAL selfCallSignal(){std::cout<<"b";}
    void SLOT selfCallSlot()
    {
        observerCallback();
        if(mEmitObservedSignal)
            emit(SIGLOT(Observer::observed));
        if(mIsActive)
            emit(SIGLOT(Observer::selfCallSignal));
    }
    bool mIsActive;
    bool mEmitObservedSignal;
};


#endif
