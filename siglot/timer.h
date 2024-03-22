#ifndef SIGLOT_TIMER_H
#define SIGLOT_TIMER_H

#include "observer.h"
namespace siglot
{
class Timer : public Observer
{
public:
    const static int TTL_INFINITE = -1;

    Timer()
    {
        setName("timer");
        mPeriod = std::chrono::seconds(1);
        mTimeToLive = TTL_INFINITE;
    }

    SIGNAL SIGNAL_timeout(){};

    SLOT setTimeToLive(int ttl)
    {
        mTimeToLive = ttl;
    }

    SLOT setTimeout(std::chrono::high_resolution_clock::duration period)
    {
        mPeriod = period;
    }

private:
    void observerCallback() override
    {
        auto timeNow = std::chrono::high_resolution_clock::now();

        //std::cout<<"timer cound: "<<std::chrono::duration_cast<std::chrono::milliseconds>((timeNow - mLastTimeoutTime)).count()
        //<<"/"
        //<<std::chrono::duration_cast<std::chrono::milliseconds>(mPeriod).count()<<std::endl;

        if ((mTimeToLive < 0 || 0 < mTimeToLive) && (timeNow - mLastTimeoutTime) >= mPeriod)
        {
            mLastTimeoutTime = timeNow;
            if(0 < mTimeToLive)
                mTimeToLive--;

            emit(SIGLOT(Timer::SIGNAL_timeout));
        }
    }

    std::chrono::high_resolution_clock::duration mPeriod;
    std::chrono::high_resolution_clock::time_point mLastTimeoutTime; // negative for infinite ttl
    int mTimeToLive;
protected:
    void onStart() override
    {
        mLastTimeoutTime = std::chrono::high_resolution_clock::now();
        std::cout<<"timer on start "<<std::endl;
    }
};
}
#endif
