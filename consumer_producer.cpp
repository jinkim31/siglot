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
        std::cout<<"producer signal"<<std::endl;
        emit(SIGLOT(Producer::ready));
    }
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
        std::cout<<"Consumer slot"<<std::endl;
    }
};

int main()
{
    EThread producerThread1, producerThread2;
    EThread consumerThread;

    Producer producer1, producer2;
    Consumer consumer;

    producer1.setName("producer 1");
    producer2.setName("producer 2");
    consumer.setName("consumer");

    producer1.move(producerThread1);
    producer2.move(producerThread2);
    consumer.move(consumerThread);

    EObject::connect(&producer1, SIGLOT(Producer::ready), &consumer, SIGLOT(Consumer::process));
    EObject::connect(&producer2, SIGLOT(Producer::ready), &consumer, SIGLOT(Consumer::process));

    producerThread1.start();
    producerThread2.start();
    consumerThread.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ELookup::instance().dumpConnectionGraph("", true);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    producerThread1.stop();
    producerThread2.stop();
    consumerThread.stop();



    producer1.remove();
    producer2.remove();
    consumer.remove();
}
