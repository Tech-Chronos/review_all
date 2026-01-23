#pragma once
#include <cstdint>
#include <functional>
#include <sys/epoll.h>

class EventLoop;
class Poller;               //  前置声明
using EventCallback = std::function<void()>;

class Channel {
public:
    Channel(int fd, EventLoop* loop);

    int Fd() const;
    uint32_t GetEvents() const;
    void SetRevents(uint32_t revents);

    bool Readable();
    bool Writable();
    void EnableRead();
    void EnableWrite();
    void DisableRead();
    void DisableWrite();
    void DisableAll();
    void Remove();
    
    void SetWriteCb(const EventCallback& cb);
    void SetReadCb (const EventCallback& cb);
    void SetErrorCb(const EventCallback& cb);
    void SetCloseCb(const EventCallback& cb);
    void SetAnyCb  (const EventCallback& cb);

    void HandleEvents(); 

private:
    void Update();

private:
    int _fd;
    uint32_t _events;
    uint32_t _revents;
    EventLoop* _loop;         

    EventCallback _write_cb, _read_cb, _error_cb, _close_cb, _any_cb;
};
