#include "Acceptor.h"

Acceptor::Acceptor(uint16_t port, EventLoop* loop)
    : _loop(loop)
    , _socket(CreateSocket(port))
    , _channel(_socket.Fd(), _loop)
{
    _channel.SetReadCb(std::bind(&Acceptor::AcceptFunc, this));
}

int Acceptor::CreateSocket(int port)
{
    assert(_socket.ServerInit(port));
    int lis_fd = _socket.Fd();
    return lis_fd;
}

void Acceptor::SetAcceptCallback(const AcceptorCallback& accept_cb)
{
    _accept_cb = accept_cb;
}

void Acceptor::Listen()
{
    _channel.EnableRead();
}

void Acceptor::AcceptFunc()
{
    int newfd = _socket.Accept();
    if (newfd < 0) 
    {
        ERR_LOG("recv newfd error!");
        exit(-1);
    }
    if (_accept_cb)
        _accept_cb(newfd);
}