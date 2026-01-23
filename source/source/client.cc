#include "Common.hpp"
#include "Socket.hpp"
#include <unordered_map>
#include <string>
#include <unistd.h>

int main()
{
    Socket client;
    client.ClientInit(8888, "127.0.0.1");

    std::string mess("I am muduo client");

    for (int i = 0; i < 5; ++i)
    {
        client.SendNonBlock(mess.c_str(), mess.size());
        sleep(1);
    }
    sleep(10);
}