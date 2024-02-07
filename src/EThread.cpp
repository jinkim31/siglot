#include "EThread.h"
#include "ELookup.h"
#include "EObject.h"

EThread::EThread()
{
    mEventLoopBreakFlag = false;
}

EThread::~EThread()
{
    stop();
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
    std::shared_lock<std::shared_mutex> lookupLock(ELookup::instance().getGlobalMutex());
    std::unique_lock<std::shared_mutex> lock(mMutex);
    size_t nQueued = mEventQueue.size();
    lock.unlock();

    for(int i=0; i<nQueued; i++)
    {
        lock.lock();
        // TODO: check if the slot object exists in this thread to this point(it might have been moved to another thread).
        auto front = std::move(mEventQueue.front());
        mEventQueue.pop();
        lock.unlock();

        if(!front.first->mThreadInAffinity)
        {
            std::cerr<<"EXPECTED! Slot object not assigned to thread."<<std::endl;
            continue;
        }
        if(front.first->mThreadInAffinity!=this)
        {
            std::cout<<"EXPECTED! Queued event's thread mismatch. This could happen on object move or remove(slot object: "<<front.first<<", this thread: "<<this<<", slot thread: "<<front.first->mThreadInAffinity<<")."<<std::endl;
            continue;
        }
        front.second();
    }
}

void EThread::pushEvent(EObject *slotObject, std::function<void(void)> &&event)
{
    std::unique_lock<std::shared_mutex> lock(mMutex);
    mEventQueue.push(std::move(std::make_pair(slotObject, std::move(event))));
}
