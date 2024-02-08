#include <iostream>
#include <siglot/object.h>
#include <siglot/timer.h>

class Producer : public Object
{
public:
    Producer()
    {
        mTimer.setTimeout(std::chrono::microseconds((int)1000000.0/60));
        connect(&mTimer, SIGLOT(Timer::timeout), this, SIGLOT(Producer::produce));
    }
    void SIGNAL ready(){}
    void SLOT produce()
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
    void SLOT consume()
    {
        std::cout<<"Consumer slot"<<std::endl;
    }
};

int main()
{
    Thread producerThread1, producerThread2;
    Thread consumerThread;

    producerThread1.setName("producer thread 1");
    producerThread2.setName("producer thread 2");
    consumerThread.setName("consumer thread");

    Producer producer1, producer2;
    Consumer consumer;

    producer1.setName("producer 1");
    producer2.setName("producer 2");
    consumer.setName("consumer");

    producer1.move(producerThread1);
    producer2.move(producerThread2);
    consumer.move(consumerThread);

    Object::connect(&producer1, SIGLOT(Producer::ready), &consumer, SIGLOT(Consumer::consume));
    Object::connect(&producer2, SIGLOT(Producer::ready), &consumer, SIGLOT(Consumer::consume));

    producerThread1.start();
    producerThread2.start();
    consumerThread.start();

    std::this_thread::sleep_for(std::chrono::seconds(5));
    Lookup::instance().dumpConnectionGraph("png", "consumer_producer.png");

    producerThread1.stop();
    producerThread2.stop();
    consumerThread.stop();

    producer1.remove();
    producer2.remove();
    consumer.remove();
}
