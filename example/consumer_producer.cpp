#include <iostream>
#include <siglot/object.h>
#include <siglot/timer.h>

using namespace siglot;

class Producer : public Object
{
public:
    Producer()
    {
        mTimer.setTimeout(std::chrono::microseconds((int)1000000.0/60));
        connect(mTimer, SIGLOT(Timer::timeout), *this, SIGLOT(Producer::produce));
    }
    SIGNAL ready(){}
    SLOT produce()
    {
        std::cout<<"producer signal"<<std::endl;
        emit(SIGLOT(Producer::ready));
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
};

class Consumer : public Object
{
public:
    SLOT consume()
    {
        std::cout<<"Consumer slot"<<std::endl;
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
