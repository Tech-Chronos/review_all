#pragma once
#include "Socket.hpp"
#include "Channel.h"
#include "EventLoop.h"
#include <cassert>

using AcceptorCallback = std::function<void(int)>; // 新的IO文件描述符到来时需要执行的回调函数
class Acceptor
{
private:
    void AcceptFunc();

    int CreateSocket(int port);
public:
    Acceptor(uint16_t port, EventLoop* loop); // 创建文件描述符，设置监听文件描述符的执行函数，放到loop中管理，

    void SetAcceptCallback(const AcceptorCallback& accept_cb);

    void Listen();
private:
    EventLoop* _loop;
    Socket _socket;
    Channel _channel; // 设置监听文件描述符关系的事件

    AcceptorCallback _accept_cb;
};