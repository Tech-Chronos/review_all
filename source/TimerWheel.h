#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <algorithm>

#include <unistd.h>
#include <sys/timerfd.h>
#include <stdint.h>

using task_t = std::function<void()>;
using release_t = std::function<void()>;

class Channel;
class EventLoop;
class TimerTask
{
public:
    TimerTask(uint64_t id, uint32_t timeout, task_t task);
    void SetRelease(release_t release);
    uint32_t GetTimeOut();
    void Cancel();
    ~TimerTask();

private:
    uint64_t _id;
    uint32_t _timeout;
    task_t _task_cb;
    release_t _release;
    bool _cancel;
};

class TimerWheel
{
private:
    void RemoveTimer(uint64_t id);
    static int CreateTimerFd();
    void TimerRead();
    void AddTimer(uint64_t id, uint32_t timeout, task_t task);
    void RefreshTimer(uint64_t id);
    void SetTimerCancel(uint64_t id);
    void RunTimer();

public:
    TimerWheel(EventLoop* loop, int capacity = 512);
    void TimerCb();
    void AddTimerInLoop(uint64_t id, uint32_t timeout, task_t task);
    void RefreshTimerInLoop(uint64_t id);
    void SetTimerCancelInLoop(uint64_t id);
    ~TimerWheel();

private:
    int _capacity;
    int _tick;
    std::vector<std::vector<std::shared_ptr<TimerTask>>> _wheel;
    std::shared_ptr<TimerTask> _tsp;
    std::unordered_map<uint64_t, std::weak_ptr<TimerTask>> _timers;

    EventLoop* _loop;  // 属于哪个loop
    int _timerfd;      // 定时器文件描述符
    Channel* _timer_channel;    //将定时器文件描述符放到loop中的poll管理
};