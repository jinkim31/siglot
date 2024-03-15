#include <siglot/object.h>
#include <siglot/timer.h>

class Server : public siglot::Object
{
public:
    std::string query(std::string&& queryStr){
        std::cout<<"Slot Server::SLOT_query called. Query"<<queryStr<<std::endl;
        std::string result = "query result";
        return result;
    }
    SIGLOT_ADD_FROM_FUNC(std::string, Server, query, std::string, queryStr)
};

class Client : public siglot::Object
{
public:
    Client()
    {
        mTimer.setTimeout(std::chrono::seconds(0));
        mTimer.setTimeToLive(1);
        connect(mTimer, SIGLOT(siglot::Timer::timeout), *this, SIGLOT(Client::SLOT_timerCallback));
    }
    SIGNAL SIGNAL_queryRequested(std::string&& query){}
    SLOT SLOT_nofifyQueryResult(std::string&& result)
    {
        std::cout<<"Client::notifyQueryResult called. Query result: "<<result<<std::endl;
    }
    void SLOT_timerCallback()
    {
        std::string queryStr = "query string";
        emit(SIGLOT(Client::SIGNAL_queryRequested), std::move(queryStr));
    }
protected:
    void onMove(siglot::Thread &thread) override
    {
        mTimer.move(thread);
        callSlot(mTimer, SIGLOT(siglot::Timer::start));
    }

    void onRemove() override
    {
        mTimer.remove();
    }
private:
    siglot::Timer mTimer;
};

int main()
{
    Server server;
    Client client;

    siglot::Object::connect(client, SIGLOT(Client::SIGNAL_queryRequested), server, SIGLOT(Server::SLOT_query));
    siglot::Object::connect(server, SIGLOT(Server::SIGNAL_query), client, SIGLOT(Client::SLOT_nofifyQueryResult));

    siglot::Thread mainThread;
    server.move(mainThread);
    client.move(mainThread);

    mainThread.step();
    mainThread.step();

    server.remove();
    client.remove();

    return 0;
}