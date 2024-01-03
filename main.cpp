#include <iostream>
#include "EObject.h"
#include "EThread.h"

class Producer : public EObject
{
public:
    void signal(std::string){}
    void emitSignal()
    {
        emit(&Producer::signal, std::string("hello"));
    }
};

class Consumer : public EObject
{
public:
    void slot(std::string str)
    {
        std::cout<<"consumer slot "<<str<<std::endl;
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

    EObject::connect(&producer, &Producer::signal, &consumer, &Consumer::slot);
    EObject::connect(&producer, &Producer::signal, &consumer, &Consumer::slot);

    consumerThread.start();

    producer.emitSignal();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    consumerThread.stop();
    producer.remove();
    consumer.remove();
}
