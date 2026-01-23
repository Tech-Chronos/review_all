#pragma once
#include "TimerWheel.h"
#include "Poller.h"
#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
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
    void AssertInLoop();
    void QueueInLoop(const Functor& cb);
    // 判断当前的任务是否在当前线程中，是就执行，不是就插入队列
    void RunInLoop(const Functor& cb);
    void UpdateEvents(Channel* channel);
    void RemoveEvent(Channel* channel);
    void Start();

    void AddTimer(uint64_t id, uint32_t timeout, task_t task);
    void RefreshTimer(uint64_t id);
    void CancelTimer(uint64_t id);
    bool HasTimer(uint64_t id);

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

class ThreadLoop
{
private:
    void ThreadEntry();
public:
    ThreadLoop();

    // 为了避免线程创建出来，但是Eventloop还没创建，此时线程就调用下面的函数，获取Eventloop指针
    EventLoop* GetEventLoopPtr();
private:
    std::mutex _mutex;
    std::condition_variable _cond;
    EventLoop* _loop; // 需要在线程内部初始化
    std::thread _thread; // 创建Eventloop的线程
};