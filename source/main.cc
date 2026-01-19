#include "Common.hpp"
#include "Buffer.hpp"
#include "Socket.hpp"
#include "Channel.h"
#include "Poller.h"
#include "EventLoop.h"
#include "TimerWheel.h"

int main()
{
    EventLoop eventloop;
    
    return 0;
}

// int main()
// {
//     Buffer buf;

//     for (int i = 0; i < 200 ; ++i)
//     {
//         std::string str = std::to_string(i) + " hello buffer!" + "\n";
//         buf.WriteStringAndPush(str);
//     }

//     while (buf.GetReadableDataNum() > 0)
//     {
//         std::string read = buf.GetLineAndPop();
//         std::cout << read << std::endl;
//     }
//     // std::string write = "hello buffer!";
//     // buf.WriteStringAndPush(write);

//     // Buffer buf1;
//     // buf1.WriteBufferAndPush(buf);

//     // std::string read = buf.ReadStringAndPop(write.size());
//     // std::string read1 = buf1.ReadStringAndPop(write.size());

//     // std::cout << read << std::endl;
//     // std::cout << read1 << std::endl;
//     return 0;
// }