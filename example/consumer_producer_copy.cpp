/*
 * This example shows copy emission of signal where the argument of Object::emit() is called by value and copied.
 * The copy emission is useful when the argument is small, and you don't want to move semantic.
 * In the process of signal emission and slot call, the argument is copied once.
 */

#include <iostream>
#include <siglot/object.h>
#include <siglot/timer.h>

using namespace siglot;

class Producer : public Object
{
public:
    Producer()
    {
        mTimer.setTimeout(std::chrono::seconds(1));
        connect(mTimer, SIGLOT(Timer::timeout), *this, SIGLOT(Producer::produce));
    }
    SIGNAL ready(PassProbe&& probe){}
protected:
    void onMove(Thread &thread) override
    {
        mTimer.move(thread);
        callSlot(mTimer, SIGLOT(Timer::start));
    }

    void onRemove() override
    {
        mTimer.remove();
        mTimer.stop();
    }
private:
    SLOT produce()
    {
        std::cout<<"producer signal emitting"<<std::endl;
        PassProbe probe(0);
        emit(SIGLOT(Producer::ready), probe);
        std::cout<<"producer signal emitted"<<std::endl;
    }
    Timer mTimer;
};

class Consumer : public Object
{
public:
    SLOT consume(PassProbe&& probe)
    {
        std::cout<<"Consumer slot called"<<std::endl;
    }
};

int main()
{
    Thread producerThread;
    Thread consumerThread;

    producerThread.setName("producer thread");
    consumerThread.setName("consumer thread");

    Producer producer;
    Consumer consumer;

    producer.setName("producer");
    consumer.setName("consumer");

    producer.move(producerThread);
    consumer.move(consumerThread);

    Object::connect(producer, SIGLOT(Producer::ready), consumer, SIGLOT(Consumer::consume));

    producerThread.start();
    consumerThread.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    Lookup::instance().dumpConnectionGraph("png", "consumer_producer_copy.png");

    producerThread.stop();
    consumerThread.stop();

    producer.remove();
    consumer.remove();
}
