#ifndef SIGLOT_THREAD_H
#define SIGLOT_THREAD_H

#include <iostream>
#include <thread>
#include <shared_mutex>
#include <atomic>
#include <functional>
#include <queue>

namespace siglot
{
class Object;

class Thread
{
public:
    Thread();
    ~Thread();
    void start();
    void stop();
    void step();
    void handleEvents();
    void setName(const std::string &name);

private:
    std::thread mThread;
    std::string mName;
    std::shared_mutex mMutex;
    std::atomic<bool> mEventLoopBreakFlag;
    std::queue<std::pair<Object *, std::function<void(void)>>> mEventQueue;
    bool mIsStepping;
    static void *entryPoint(void *param);
    void pushEvent(Object *slotObject, std::function<void(void)> &&event);
    void runEventLoop();

    friend class Object;

    friend class Lookup;
};
}
#endif
