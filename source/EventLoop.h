#pragma once
#include "TimerWheel.h"
#include "Poller.h"
#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <sys/eventfd.h>
#include <stdint.h>

class Channel;
class Poller;
class TimerWheel;
using Functor = std::function<void()>;

class EventLoop
{
public:
    void RunAllTask();
public:
    EventLoop();
    bool IsInLoop();
    void QueueInLoop(const Functor& cb);
    // 判断当前的任务是否在当前线程中，是就执行，不是就插入队列
    void RunInLoop(const Functor& cb);
    void UpdateEvents(Channel* channel);
    void RemoveEvent(Channel* channel);
    void Start();

    void AddTimer(uint64_t id, uint32_t timeout, task_t task);
    void RefreshTimer(uint64_t id);
    void CancelTimer(uint64_t id);

private:
    int CreateEventfd();
    void SetEventCb();
    void WakeUpEventFd();

private:
    std::thread::id _thread_id;
    int _eventfd; // 用于事件通知的文件描述符
    std::unique_ptr<Channel> _event_channel;
    Poller _poll;
    std::vector<Functor> _tasks; // 任务队列
    std::mutex _mtx;
    bool _isrunning;
    TimerWheel _wheel;
};