#ifndef SIGLOT_TIMER_H
#define SIGLOT_TIMER_H

#include "observer.h"
namespace siglot
{
class Timer : public Observer
{
public:
    Timer()
    {
        setName("timer");
        mPeriod = std::chrono::seconds(1);
    }

    void SIGNAL timeout()
    {};

    void SLOT setTimeout(const std::chrono::high_resolution_clock::duration &period)
    {
        mPeriod = period;
    }

private:
    void observerCallback() override
    {
        auto timeNow = std::chrono::high_resolution_clock::now();
        if ((timeNow - mLastTimeoutTime) >= mPeriod)
        {
            emit(SIGLOT(Timer::timeout));
            mLastTimeoutTime = timeNow;
        }
    }

    std::chrono::high_resolution_clock::duration mPeriod;
    std::chrono::high_resolution_clock::time_point mLastTimeoutTime;
protected:
    void onStart() override
    {
        mLastTimeoutTime = std::chrono::high_resolution_clock::now();
    }
};
}
#endif
