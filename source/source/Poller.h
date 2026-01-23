#pragma once
#include "Common.hpp"
#include <unordered_map>
#include <vector>
#include <sys/epoll.h>
#define SIZE 1024

class Channel; //  前置声明

class Poller
{
public:
    Poller();
    ~Poller();

    void UpdateEvents(Channel *channel);
    void RemoveEvent(Channel *channel);

    //  返回活跃 Channel 列表给上层处理
    void Poll(std::vector<Channel *> &active);

private:
    bool IfExist(int fd) const;
    void UpdateMonitor(Channel *channel, int op);

private:
    int _epfd;
    struct epoll_event _evs[SIZE];
    std::unordered_map<int, Channel *> _care_fd;
};
