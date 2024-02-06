#include <iostream>
#include "EObject.h"
#include "EThread.h"
#include "EObserver.h"

class Producer : public EObject
{
public:
    Producer()
    {
        connect(&mObserver, SIGLOT(EObserver::observed), this, SIGLOT(Producer::observeCallback));
    }
    void SIGNAL ready(){}
    void SLOT observeCallback()
    {
        std::cout<<"produce signal emit"<<std::endl;
        emit(SIGLOT(Producer::ready));
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
    void SLOT process()
    {
        std::cout<<"Consumer slot call"<<std::endl;
    }
};

int main()
{
    EThread producerThread;
    EThread consumerThread;

    Producer producer;
    Consumer consumer;

    producer.move(producerThread);
    consumer.move(consumerThread);

    EObject::connect(&producer, SIGLOT(Producer::ready), &consumer, SIGLOT(Consumer::process));

    producerThread.start();
    consumerThread.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    producerThread.stop();
    consumerThread.stop();

    producer.remove();
    consumer.remove();
}
