#include "Poller.h"
#include "Channel.h"
#include <unistd.h>

Poller::Poller()
{
    _epfd = epoll_create(SIZE);
    if (_epfd < 0)
    {
        ERR_LOG("epoll create error!");
        std::exit(-1);
    }
}

Poller::~Poller()
{
    if (_epfd >= 0)
        close(_epfd);
}

bool Poller::IfExist(int fd) const
{
    return _care_fd.find(fd) != _care_fd.end();
}

void Poller::UpdateMonitor(Channel *channel, int op)
{
    epoll_event ev{};
    ev.data.fd = channel->Fd();
    ev.events = channel->GetEvents();
    if (epoll_ctl(_epfd, op, channel->Fd(), &ev) < 0)
    {
        ERR_LOG("epoll_ctl error!");
        std::exit(-1);
    }
}

void Poller::UpdateEvents(Channel *channel)
{
    int fd = channel->Fd();
    if (IfExist(fd))
        UpdateMonitor(channel, EPOLL_CTL_MOD);
    else
    {
        UpdateMonitor(channel, EPOLL_CTL_ADD);
        _care_fd[fd] = channel;
    }
}

void Poller::RemoveEvent(Channel *channel)
{
    int fd = channel->Fd();
    epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, nullptr);
    _care_fd.erase(fd);
}

void Poller::Poll(std::vector<Channel *> &active)
{
    int n = epoll_wait(_epfd, _evs, SIZE, -1);
    if (n < 0)
    {
        if (errno == EINTR)
            return;
        ERR_LOG("epoll_wait Error!");
        std::exit(-1);
    }
    for (int i = 0; i < n; ++i)
    {
        int fd = _evs[i].data.fd;
        auto it = _care_fd.find(fd);
        if (it == _care_fd.end())
            continue;
        Channel *ch = it->second;
        ch->SetRevents(_evs[i].events);
        active.push_back(ch);
    }
}
