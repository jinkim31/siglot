#include "EThread.h"
#include "ELookup.h"

EThread::EThread()
{
    mEventLoopBreakFlag = false;
}

void EThread::start()
{
    mThread = std::thread(EThread::entryPoint, this);
    mEventLoopBreakFlag = false;
}

void EThread::stop()
{
    mEventLoopBreakFlag = true;
    if(mThread.joinable())
        mThread.join();
}

void *EThread::entryPoint(void *param)
{
    auto* ethreadPtr = (EThread*)param;
    ethreadPtr->runEventLoop();
    return nullptr;
}

void EThread::runEventLoop()
{
    while(true)
    {
        if (mEventLoopBreakFlag) // atomic
            return;

        step();
    }
}

void EThread::step()
{
    std::unique_lock<std::shared_mutex> lock(mMutex);
    size_t nQueued = mEventQueue.size();
    lock.unlock();
    //std::cout<<nQueued<<" events in queue."<<std::endl;
    for(int i=0; i<nQueued; i++)
    {
        lock.lock();
        auto front = std::move(mEventQueue.front());
        mEventQueue.pop();
        lock.unlock();
        front();
    }
}

void EThread::pushEvent(std::function<void(void)> &&event)
{
    std::unique_lock<std::shared_mutex> lock(mMutex);
    mEventQueue.push(std::move(event));
}

