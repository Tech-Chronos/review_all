#include "../source/Common.hpp"
#include "../source/Socket.hpp"

#include <vector>
#include <string>
#include <cerrno>
#include <unistd.h>

int main()
{
    Socket listensock;
    listensock.ServerInit(8888);

    while (true)
    {
        int conn = listensock.Accept();
        if (conn >= 0)
        {
            Socket connfd(conn);

            char buffer[1024];

            int ret = connfd.Recv(buffer, 1023);
            if (ret > 0)
            {
                buffer[ret] = 0;
                INF_LOG("client say: %s", buffer);
            }
            std::string message = "server echo: ";
            message += buffer;

            connfd.Send(message.c_str(), message.size());
        }
        sleep(1);
    }

    return 0;
}
