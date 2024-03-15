#include <siglot/object.h>
#include <siglot/timer.h>

#define SIGLOT_ADD_FROM_FUNC(className, retType, funcName, paramTypes) \
void SIGNAL_##funcName ( retType && ){}; \
void SLOT_##funcName paramType { emit( #className "::" #funcName, className :: funcName , std::move(funcName())) }


class Server : public siglot::Object
{
public:
    std::string query(std::string&& query, int&& seq){
        std::cout<<"Slot Server::SLOT_query called. Query string: "<<query<<std::endl;
        std::string result = "query result";
        emit(SIGLOT(Server::SIGNAL_queryRetutn), std::move(result));
    }

    SIGNAL SIGNAL_queryRetutn(std::string&& result){}
    SLOT SLOT_query(std::string&& query, int&& seq){
        std::cout<<"Slot Server::SLOT_query called. Query string: "<<query<<std::endl;
        std::string result = "query result";
        emit(SIGLOT(Server::SIGNAL_queryRetutn), std::move(result));
    }
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
    SIGNAL SIGNAL_queryRequested(std::string&& query, int&& seq){}
    SLOT SLOT_nofifyQueryResult(std::string&& result)
    {
        std::cout<<"Client::notifyQueryResult called. Query result: "<<result<<std::endl;
    }
    void SLOT_timerCallback()
    {
        std::string queryStr = "query string";
        emit(SIGLOT(Client::SIGNAL_queryRequested), std::move(queryStr), 123);
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
    siglot::Object::connect(server, SIGLOT(Server::SIGNAL_queryRetutn), client, SIGLOT(Client::SLOT_nofifyQueryResult));

    siglot::Thread mainThread;
    server.move(mainThread);
    client.move(mainThread);

    mainThread.step();
    mainThread.step();

    server.remove();
    client.remove();

    return 0;
}