#pragma once

#include "Buffer.hpp"
#include "Socket.hpp"
#include "Channel.h"
#include "EventLoop.h"

#include <iostream>
#include <memory>
#include <functional>

class Connection;

typedef enum
{
    CONNECTING,
    CONNECTED,
    DISCONNECTING,
    DISCONNECTED
} STATUS;

using MessageCallBack = std::function<void(std::shared_ptr<Connection>, Buffer* buf)>;
using AnyEventCallBack = std::function<void(std::shared_ptr<Connection>)>;
using ConnectedCallBack = std::function<void(std::shared_ptr<Connection>)>;
using CloseCallBack = std::function<void(std::shared_ptr<Connection>)>;

class Connection : public std::enable_shared_from_this<Connection>
{
public:
    Connection(EventLoop* loop, MessageCallBack message_cb, 
               AnyEventCallBack any_cb, ConnectedCallBack conn_cb,
               CloseCallBack close_cb, CloseCallBack server_close_cb);
private:
    // 5个事件处理函数
    void HandleRead();
    void HandleWrite();
    void HandleClose();
    void HandleError();
    void HandleAnyEvent();

    void ShutDown();
    void EstablishedInLoop();
    void ReleaseInLoop();

    // 在eventloop中取消非活跃事件的释放
    void CancelInactiveEventReleaseInLoop();
    
private:
    uint64_t _conn_id; // 表示当前连接的id
    int _fd; // 当前这个连接的文件描述符
    Socket _socket;
    EventLoop* _loop;
    Channel _channel; // 当前这个连接关心的事件以及回调函数
    bool _enable_inactive_release; // 非活跃链接的释放是否开启

    Buffer _in_buffer;
    Buffer _out_buffer;
    STATUS _status; // 当前这个链接的状态

    MessageCallBack _message_cb; // 事件到来的时候的业务处理回调函数
    AnyEventCallBack _any_cb; // 处理任意事件的回调函数
    ConnectedCallBack _conn_cb; // 刚建立链接的回调函数
    CloseCallBack _close_cb; // 链接关闭的客户端回调函数
    CloseCallBack _server_close_cb; // 链接关闭的服务器回调函数
};