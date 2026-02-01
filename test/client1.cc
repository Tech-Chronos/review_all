#include "../source/Socket.hpp"
#include <string>
#include <cassert>
// 测试1：验证长链接是否可行
// int main()
// {
//     Socket cli_sock;
//     cli_sock.ClientInit(8888, "127.0.0.1");
//     std::string req = "GET /hello HTTP/1.1\r\nConnection: keep-alive\r\n\r\n";
//     while (true)
//     {
//         cli_sock.SendNonBlock(req.c_str(), req.size());
//         char buffer[1024];
//         cli_sock.RecvNonBlock(buffer, 1023);
//         std::cout << buffer << std::endl;
//         sleep(1);

//     }
// }

// 验证长链接不发送消息
int main()
{
    Socket cli_sock;
    cli_sock.ClientInit(8888, "127.0.0.1");
    std::string req = "GET /hello HTTP/1.1\r\nConnection: keep-alive\r\nContent-Length: 100\r\n\r\nhello muduo";
    std::cout << req.size() << std::endl;
    while (1)
    {
        assert(cli_sock.SendNonBlock(req.c_str(), req.size()) != -1);
        assert(cli_sock.SendNonBlock(req.c_str(), req.size()) != -1);
        assert(cli_sock.SendNonBlock(req.c_str(), req.size()) != -1);
        char buf[1024] = {0};
        assert(cli_sock.RecvNonBlock(buf, 1023));
        DBG_LOG("[%s]", buf);
        sleep(3);
    }
    cli_sock.Close();
    return 0;
}