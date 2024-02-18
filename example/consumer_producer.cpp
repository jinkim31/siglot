/*
 * This example shows basic use of threads, objects, and signal-slot pattern.
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
        mCounter = 0;
        mTimer.setTimeout(std::chrono::milliseconds(100));
        connect(mTimer, SIGLOT(Timer::timeout), *this, SIGLOT(Producer::produce));
    }
    SIGNAL ready(std::string&& str){}
    SLOT produce()
    {
        std::cout<<"producer signal: "<<mCounter<<std::endl;
        emit(SIGLOT(Producer::ready), std::to_string(mCounter++));
    }
protected:
    void onMove(Thread &thread) override
    {
        mTimer.move(thread);
        mTimer.start();
    }

    void onRemove() override
    {
        mTimer.remove();
        mTimer.stop();
    }
private:
    Timer mTimer;
    int mCounter;
};

class Consumer : public Object
{
public:
    SLOT consume(std::string&& str)
    {
        std::cout<<"Consumer slot: "<<str<<std::endl;
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

    producer.setName("producer 1");
    consumer.setName("consumer");

    producer.move(producerThread);
    consumer.move(consumerThread);

    Object::connect(producer, SIGLOT(Producer::ready), consumer, SIGLOT(Consumer::consume));

    producerThread.start();
    consumerThread.start();

    std::this_thread::sleep_for(std::chrono::seconds(5));
    Lookup::instance().dumpConnectionGraph("png", "consumer_producer.png");

    producerThread.stop();
    consumerThread.stop();

    producer.remove();
    consumer.remove();
}
