#ifndef ETHREAD2_ETHREAD_H
#define ETHREAD2_ETHREAD_H

#include <iostream>
#include <thread>
#include <shared_mutex>
#include <atomic>
#include <functional>
#include <queue>

class EThread
{
public:
    EThread();
    ~EThread();
    void start();
    void stop();
    void step();
private:
    std::thread mThread;
    std::shared_mutex mMutex;
    std::atomic<bool> mEventLoopBreakFlag;
    std::queue<std::function<void(void)>> mEventQueue;
    static void* entryPoint(void* param);
    void pushEvent(std::function<void(void)>&& event);
    void runEventLoop();
friend class EObject;
};


#endif
