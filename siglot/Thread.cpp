#include "Thread.h"
#include "Lookup.h"
#include "Object.h"

Thread::Thread()
{
    mEventLoopBreakFlag = false;
}

Thread::~Thread()
{
    stop();
}

void Thread::start()
{
    mThread = std::thread(Thread::entryPoint, this);
    mEventLoopBreakFlag = false;
}

void Thread::stop()
{
    mEventLoopBreakFlag = true;
    if(mThread.joinable())
        mThread.join();
}

void *Thread::entryPoint(void *param)
{
    auto* ethreadPtr = (Thread*)param;
    ethreadPtr->runEventLoop();
    return nullptr;
}

void Thread::runEventLoop()
{
    while(true)
    {
        if (mEventLoopBreakFlag) // atomic
            return;

        step();
    }
}

void Thread::step()
{
    std::shared_lock<std::shared_mutex> lookupLock(Lookup::instance().getGlobalMutex());
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

void Thread::pushEvent(Object *slotObject, std::function<void(void)> &&event)
{
    std::unique_lock<std::shared_mutex> lock(mMutex);
    mEventQueue.push(std::move(std::make_pair(slotObject, std::move(event))));
}
