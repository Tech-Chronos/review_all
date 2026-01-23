#include "Channel.h"
#include "Poller.h"
#include "EventLoop.h"

Channel::Channel(int fd, EventLoop *loop)
    : _fd(fd), _events(0), _revents(0), _loop(loop) {}


bool Channel::Readable()
{
    return _events & EPOLLIN;
}

bool Channel::Writable()
{
    return _events & EPOLLOUT;
}

void Channel::Update()
{
    _loop->UpdateEvents(this);
}

void Channel::Remove()
{
    _loop->RemoveEvent(this);
}

uint32_t Channel::GetEvents() const
{
    return _events;
}

int Channel::Fd() const
{
    return _fd;
}

void Channel::SetRevents(uint32_t revents)
{
    _revents = revents;
}

// 使能读事件
void Channel::EnableRead()
{
    _events |= EPOLLIN;
    Update();
}

// 使能写事件
void Channel::EnableWrite()
{
    _events |= EPOLLOUT;
    Update();
}

// 取消读事件
void Channel::DisableRead()
{
    _events &= ~EPOLLIN; // 0010 1101
    Update();
}

// 取消写事件
void Channel::DisableWrite()
{
    _events &= ~EPOLLOUT;
    Update();
}

// 取消所有事件监控
void Channel::DisableAll()
{
    _events = 0;
    Update();
}

// 设置回调
void Channel::SetWriteCb(const EventCallback &cb)
{
    _write_cb = cb;
}

void Channel::SetReadCb(const EventCallback &cb)
{
    _read_cb = cb;
}

void Channel::SetErrorCb(const EventCallback &cb)
{
    _error_cb = cb;
}

void Channel::SetCloseCb(const EventCallback &cb)
{
    _close_cb = cb;
}

void Channel::SetAnyCb(const EventCallback &cb)
{
    _any_cb = cb;
}

// “错误/关闭优先  any 只一次”
void Channel::HandleEvents()
{
    if (_any_cb)
        _any_cb();

    if (_revents & EPOLLERR)
    {
        if (_error_cb)
            _error_cb();
        return;
    }
    if (_revents & EPOLLHUP)
    {
        if (_close_cb)
            _close_cb();
        return;
    }

    if (_revents & (EPOLLIN | EPOLLPRI))
    {
        if (_read_cb)
            _read_cb();
    }
    if (_revents & EPOLLOUT)
    {
        if (_write_cb)
            _write_cb();
    }
}
