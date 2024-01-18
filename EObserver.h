#ifndef ETHREAD2_EOBSERVER_H
#define ETHREAD2_EOBSERVER_H

#include "EObject.h"
#include "ELookup.h"
#include <chrono>

class EObserver : public EObject
{
public:
    EObserver(bool emitObservedSignal = false)
    {
        mIsActive = false;
        mEmitObservedSignal = emitObservedSignal;
        //TODO: this line outputs error: EXC_BAD_ACCESS connect(this, &EObserver::selfCallSignal, this, &EObserver::selfCallSlot, EConnection::DIRECT);
        connect(this, &EObserver::selfCallSignal, this, &EObserver::selfCallSlot, EConnection::QUEUED);
    }
    void start()
    {
        if(mIsActive)
            return;
        mIsActive = true;
        emit(&EObserver::selfCallSignal);
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
        std::cout<<"EObserver callback"<<std::endl;
    }
private:
    void SIGNAL selfCallSignal(){std::cout<<"b";}
    void SLOT selfCallSlot()
    {
        observerCallback();
        //if(mEmitObservedSignal)
        //    emit(&EObserver::observed);
        if(mIsActive)
            emit(&EObserver::selfCallSignal);
    }
    bool mIsActive;
    bool mEmitObservedSignal;
};


#endif
