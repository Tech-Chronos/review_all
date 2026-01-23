#include "TimerWheel.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Common.hpp"

// TimerTask
TimerTask::TimerTask(uint64_t id, uint32_t timeout, task_t task)
    : _id(id), _timeout(timeout), _task_cb(task), _cancel(false)
{
}

void TimerTask::SetRelease(release_t release)
{
    _release = release;
}

uint32_t TimerTask::GetTimeOut()
{
    return _timeout;
}

void TimerTask::Cancel()
{
    _cancel = true;
}

TimerTask::~TimerTask()
{
    INF_LOG("Timer task delete");
    if (!_cancel && _task_cb)
        _task_cb();
    if (_release)
        _release();
}

// TimerWheel
// private
void TimerWheel::RemoveTimer(uint64_t id)
{
    std::unordered_map<uint64_t, std::weak_ptr<TimerTask>>::iterator it = _timers.find(id);
    if (it != _timers.end())
    {
        _timers.erase(it);
    }
}

int TimerWheel::CreateTimerFd()
{
    int timefd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timefd < 0)
    {
        ERR_LOG("timerfd_create error!");
        exit(-1);
    }

    struct itimerspec its = {0};
    its.it_value.tv_sec = 1;
    its.it_value.tv_nsec = 0;

    its.it_interval.tv_sec = 1;
    its.it_interval.tv_nsec = 0;

    timerfd_settime(timefd, 0, &its, nullptr);

    return timefd;
}

void TimerWheel::AddTimer(uint64_t id, uint32_t timeout, task_t task)
{
    std::shared_ptr<TimerTask> tsp = std::make_shared<TimerTask>(id, timeout, task);

    tsp->SetRelease(std::bind(&TimerWheel::RemoveTimer, this, id));

    size_t pos = (_tick + timeout) % _capacity;
    _wheel[pos].push_back(tsp);

    _timers[id] = std::weak_ptr<TimerTask>(tsp);
}

void TimerWheel::RefreshTimer(uint64_t id)
{
    std::unordered_map<uint64_t, std::weak_ptr<TimerTask>>::iterator it = _timers.find(id);
    if (it != _timers.end())
    {
        std::shared_ptr<TimerTask> tsp(_timers[id].lock());
        if (!tsp)
            return;
        size_t pos = (_tick + it->second.lock()->GetTimeOut()) % _capacity;
        _wheel[pos].push_back(tsp);
    }
    return;
}

void TimerWheel::SetTimerCancel(uint64_t id)
{
    std::unordered_map<uint64_t, std::weak_ptr<TimerTask>>::iterator it = _timers.find(id);
    if (it == _timers.end())
        return;

    std::shared_ptr<TimerTask> tsp(_timers[id].lock());
    if (!tsp)
        return;
    tsp->Cancel();
    std::cout << "id :" << id << " 被取消执行了！" << std::endl;
}

// 清空timer
void TimerWheel::TimerRead()
{
    uint64_t expire = 0;
    int ret = read(_timerfd, &expire, sizeof(expire));
}

// public
// 定时器++，清空tick位置的定时任务
void TimerWheel::RunTimer()
{
    _tick = (_tick + 1) % _capacity;
    _wheel[_tick].clear();
}

// 定时触发回调，epoll中有epollin，调用这个函数
void TimerWheel::TimerCb()
{
    TimerRead();
    RunTimer();
}

TimerWheel::TimerWheel(EventLoop* loop, int capacity)
    : _capacity(capacity)
    , _tick(0)
    , _loop(loop)
    , _timerfd(CreateTimerFd())
{
    // resize会调用对应位置的构造函数
    _wheel.resize(capacity);

    // 设置timerfd进入loop的epoll，并且设置他的回调函数，使能读
    _timer_channel = new Channel(_timerfd, _loop);
    _timer_channel->SetReadCb(std::bind(&TimerWheel::TimerCb, this));
    _timer_channel->EnableRead();
}


// 业务线程调用的时候会将任务插入到对应eventloop的任务队列
// 然后将eventfd的epoll唤醒，进行runalltask
void TimerWheel::AddTimerInLoop(uint64_t id, uint32_t timeout, task_t task)
{
    _loop->RunInLoop(std::bind(&TimerWheel::AddTimer, this, id, timeout, task));
}

void TimerWheel::RefreshTimerInLoop(uint64_t id)
{
    _loop->RunInLoop(std::bind(&TimerWheel::RefreshTimer, this, id));
}

void TimerWheel::SetTimerCancelInLoop(uint64_t id)
{
   _loop->RunInLoop(std::bind(&TimerWheel::SetTimerCancel, this, id));
}

bool TimerWheel::HasTimerId(uint64_t id)
{
    auto pos = _timers.find(id);
    return (pos != _timers.end());
}


TimerWheel::~TimerWheel()
{
}