#ifndef ETHREAD2_ETHREAD_H
#define ETHREAD2_ETHREAD_H

#include <iostream>
#include <thread>
#include <shared_mutex>
#include <atomic>
#include <functional>
#include <queue>

class Object;

class Thread
{
public:
    Thread();
    ~Thread();
    void start();
    void stop();
    void step();
private:
    std::thread mThread;
    std::shared_mutex mMutex;
    std::atomic<bool> mEventLoopBreakFlag;
    std::queue<std::pair<Object*, std::function<void(void)>>> mEventQueue;
    static void* entryPoint(void* param);
    void pushEvent(Object *slotObject, std::function<void(void)> &&event);
    void runEventLoop();
friend class Object;
};


#endif
