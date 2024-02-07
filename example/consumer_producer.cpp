#include <iostream>
#include <siglot/object.h>
#include <siglot/observer.h>

class Producer : public Object
{
public:
    Producer()
    {
        connect(&mObserver, SIGLOT(Observer::observed), this, SIGLOT(Producer::observeCallback));
    }
    void SIGNAL ready(){}
    void SLOT observeCallback()
    {
        std::cout<<"producer signal"<<std::endl;
        emit(SIGLOT(Producer::ready));
    }
protected:
    void onMove(Thread &thread) override
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
    Observer mObserver;
};

class Consumer : public Object
{
public:
    void SLOT process()
    {
        std::cout<<"Consumer slot"<<std::endl;
    }
};

int main()
{
    Thread producerThread1, producerThread2;
    Thread consumerThread;

    Producer producer1, producer2;
    Consumer consumer;

    producer1.setName("producer 1");
    producer2.setName("producer 2");
    consumer.setName("consumer");

    producer1.move(producerThread1);
    producer2.move(producerThread2);
    consumer.move(consumerThread);

    Object::connect(&producer1, SIGLOT(Producer::ready), &consumer, SIGLOT(Consumer::process));
    Object::connect(&producer2, SIGLOT(Producer::ready), &consumer, SIGLOT(Consumer::process));

    producerThread1.start();
    producerThread2.start();
    consumerThread.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    Lookup::instance().dumpConnectionGraph("", true);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    producerThread1.stop();
    producerThread2.stop();
    consumerThread.stop();



    producer1.remove();
    producer2.remove();
    consumer.remove();
}
