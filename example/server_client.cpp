#include <siglot/object.h>
#include <siglot/thread.h>
#include <siglot/timer.h>

class Server : public siglot::Object
{
public:
    SLOT addRequest(std::string str)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        mDatabase.push_back(str);
        emit(SIGLOT(Server::addResponded), true);
    }
    SIGNAL addResponded(bool successful){}
private:
    std::vector<std::string> mDatabase;
};

class Client : public siglot::Object
{
public:
    Client()
    {
        connect(mTimer, SIGLOT(siglot::Timer::timeout), *this, SIGLOT(Client::timerCallback));
    }
    SIGNAL addRequested(std::string str){}
    SLOT addRespond(bool successful)
    {
        std::cout<<"add successful: "<<successful<<std::endl;
    }
private:
    siglot::Timer mTimer;
    SLOT timerCallback()
    {

    }
protected:
    void onMove(siglot::Thread &thread) override
    {
        mTimer.move(thread);
        callSlot(mTimer, SIGLOT(siglot::Timer::setTimeout), (std::chrono::high_resolution_clock::duration)std::chrono::milliseconds(1000));
        callSlot(mTimer, SIGLOT(siglot::Timer::start));
    }

    void onRemove() override
    {
        mTimer.stop();
        mTimer.remove();
    }
};

int main()
{
    siglot::Thread serverThread, clientThread;
    Server server;
    Client client;
    serverThread.start();
    clientThread.start();
    server.move(serverThread);
    client.move(clientThread);

    std::this_thread::sleep_for(std::chrono::seconds(3));

    siglot::Lookup::instance().dumpConnectionGraph("png", "server_client.png");
    serverThread.stop();
    clientThread.stop();
    server.remove();
    client.remove();
}