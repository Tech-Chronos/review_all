#include "../source/Common.hpp"
#include "../source/Socket.hpp"

#include <iostream>
#include <string>
#include <unistd.h>
#include <cerrno>

int main()
{
    Socket clisock;
    clisock.ClientInit(8888, "0.0.0.0");

    std::string message;

    std::cout << "Input> ";
    std::getline(std::cin, message);

    clisock.Send(message.c_str(), message.size());

    char buffer[1024];
    int ret = clisock.Recv(buffer, 1023);
    if (ret > 0)
    {
        buffer[ret] = 0;
    }

    std::cout << buffer << std::endl;
    return 0;
}
