#ifndef ETHREAD2_EOBSERVER_H
#define ETHREAD2_EOBSERVER_H

#include "EObject.h"
#include "ELookup.h"
#include <chrono>

class EObserver : public EObject
{
public:
    EObserver(bool emitObservedSignal = true)
    {
        mIsActive = false;
        mEmitObservedSignal = emitObservedSignal;
        setName("observer");
        //TODO: this line outputs error: EXC_BAD_ACCESS connect(this, &EObserver::selfCallSignal, this, &EObserver::selfCallSlot, EConnection::DIRECT);
        connect(this, SIGLOT(EObserver::selfCallSignal), this, SIGLOT(EObserver::selfCallSlot), EConnection::QUEUED, true);
    }
    void start()
    {
        if(mIsActive)
            return;
        mIsActive = true;
        emit(SIGLOT(EObserver::selfCallSignal));
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
            emit(SIGLOT(EObserver::observed));
        if(mIsActive)
            emit(SIGLOT(EObserver::selfCallSignal));
    }
    bool mIsActive;
    bool mEmitObservedSignal;
};


#endif
