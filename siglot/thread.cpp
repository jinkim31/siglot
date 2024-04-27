#include "thread.h"
#include "lookup.h"
#include "object.h"

siglot::Thread::Thread()
{
    mEventLoopBreakFlag = false;
    mName = "thread";
    mEventLoopDelay = std::chrono::milliseconds(1);
    mHasGlobalLock = false;
}

siglot::Thread::~Thread()
{
    stop();
}

void siglot::Thread::setName(const std::string &name)
{
    std::unique_lock<std::shared_mutex> lock(Lookup::instance().getGlobalMutex());
    mName = name;
}

void siglot::Thread::start()
{
    mThread = std::thread(Thread::entryPoint, this);
    mEventLoopBreakFlag = false;
}

void siglot::Thread::stop()
{
    mEventLoopBreakFlag = true;
    if(mThread.joinable())
        mThread.join();
}

void *siglot::Thread::entryPoint(void *param)
{
    auto* ethreadPtr = (Thread*)param;
    ethreadPtr->runEventLoop();
    return nullptr;
}

void siglot::Thread::runEventLoop()
{
    while(true)
    {
        if (mEventLoopBreakFlag) // atomic
            return;

        step();
        //std::this_thread::sleep_for(mEventLoopDelay);
    }
}

void siglot::Thread::handleEvents()
{
    std::unique_lock<std::shared_mutex> lock(mMutex);
    size_t nQueued = mEventQueue.size();
    lock.unlock();

    for(int i=0; i<nQueued; i++)
    {
        // extract front event
        lock.lock();
        if(mEventQueue.empty())
            return;
        auto front = std::move(mEventQueue.front());
        mEventQueue.pop();
        lock.unlock();

        // check if the object with the id is still around
        const auto& idObjectPair = Lookup::instance().mObjectList.find(front.first);
        if(idObjectPair == Lookup::instance().mObjectList.end())
            continue;

        // check thread affinity
        if(idObjectPair->second->mThreadInAffinity!=this)
        {
            std::cout<<"EXPECTED!"<<std::endl;
            continue;
        }
        front.second();
    }
}

void siglot::Thread::step()
{
    std::shared_lock<std::shared_mutex> lookupLock(Lookup::instance().getGlobalMutex());
    mHasGlobalLock = true;
    handleEvents();
    mHasGlobalLock = false;
}

void siglot::Thread::pushEvent(size_t objectID, std::function<void(void)> &&event)
{
    std::unique_lock<std::shared_mutex> lock(mMutex);
    mEventQueue.emplace(objectID, std::move(event));
}

void siglot::Thread::setEventLoopDelay(const std::chrono::high_resolution_clock::duration &delay)
{
    std::unique_lock<std::shared_mutex> lock(mMutex);
    mEventLoopDelay = delay;
}
