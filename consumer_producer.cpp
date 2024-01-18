#include <iostream>
#include "EObject.h"
#include "EThread.h"
#include "EObserver.h"

class Producer : public EObject
{
public:
    Producer()
    {
        connect(&mObserver, &EObserver::observed, this, &Producer::observeCallback);
    }
    void SIGNAL ready(int number, std::string data){}
    void SLOT observeCallback()
    {
        std::cout<<"produce"<<std::endl;
    }

    void a(){}
    void b(){}
protected:
    void onMove(EThread &thread) override
    {
        mObserver.move(thread);
        mObserver.start();
    }

    void onRemove() override
    {
        mObserver.remove();
        mObserver.stop();
    }
private:
    EObserver mObserver;
};

class Consumer : public EObject
{
public:
    void SLOT process(int number, std::string data)
    {

    }
};

int main()
{
    void (Producer::*aPtr)() = &Producer::a;
    void (Producer::*bPtr)() = &Producer::b;

    std::uintptr_t aUint = reinterpret_cast<std::uintptr_t>(static_cast<const char*>(static_cast<const void*>(&aPtr)));
    std::uintptr_t bUint = reinterpret_cast<std::uintptr_t>(static_cast<const char*>(static_cast<const void*>(&bPtr)));

    std::cout<<aUint<<" "<<bUint<<std::endl;
    if(aPtr == aPtr)
        std::cout<<"t"<<std::endl;
    else
        std::cout<<"f"<<std::endl;
    /*
    EThread producerThread;
    EThread consumerThread;

    Producer producer;
    Consumer consumer;

    producer.move(producerThread);
    consumer.move(consumerThread);

    EObject::connect(&producer, &Producer::ready, &consumer, &Consumer::process);

    producerThread.start();
    consumerThread.start();

    //std::this_thread::sleep_for(std::chrono::milliseconds(1));

    producerThread.stop();
    consumerThread.stop();

    producer.remove();
    consumer.remove();
     */
}
