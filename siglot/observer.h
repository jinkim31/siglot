#ifndef SIGLOT_OBSERVER_H
#define SIGLOT_OBSERVER_H

#include "object.h"
#include "lookup.h"
#include <chrono>

namespace siglot
{
class Observer : public Object
{
public:
    Observer()
    {
        mIsActive = false;
        setName("observer");
        // queued connection is used even though the signal and the slot are in the same thread
        // direct connection would result in infinite selfCallSlot() recursion and stack overflow
        connect(*this, SIGLOT(Observer::selfCallSignal), *this, SIGLOT(Observer::selfCallSlot), Connection::QUEUED, true);
    }

    void start()
    {
        if (mIsActive)
            return;
        mIsActive = true;
        onStart();
        emit(SIGLOT(Observer::selfCallSignal));
    }

    void stop()
    {
        if (!mIsActive)
            return;
        mIsActive = false;
        onStop();
    }

    SIGNAL SIGNAL_observed(){}

protected:
    virtual void observerCallback(){}

    virtual void onStart()
    {}

    virtual void onStop()
    {}

private:
    SIGNAL selfCallSignal()
    {}

    SLOT selfCallSlot()
    {
        observerCallback();
        emit(SIGLOT(Observer::SIGNAL_observed));
        if (mIsActive)
            emit(SIGLOT(Observer::selfCallSignal));
    }

    bool mIsActive;
};
}
#endif
