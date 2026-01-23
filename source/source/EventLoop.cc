#include "Channel.h"
#include "Poller.h"
#include "EventLoop.h"
#include "TimerWheel.h"
#include <cassert>

void EventLoop::RunAllTask()
{
    std::vector<Functor> tasks;
    {
        std::unique_lock<std::mutex> _lock(_mtx);
        _tasks.swap(tasks);
    }
    for (auto &t : tasks)
    {
        t();
    }
}

EventLoop::EventLoop()
    : _thread_id(std::this_thread::get_id())
    , _eventfd(CreateEventfd())
    , _event_channel(std::make_unique<Channel>(_eventfd, this))
    , _isrunning(false)
    , _wheel(this)
{
    _event_channel->SetReadCb(std::bind(&EventLoop::SetEventCb, this));
    _event_channel->EnableRead();
}

bool EventLoop::IsInLoop()
{
    return (std::this_thread::get_id() == _thread_id);
}

void EventLoop::AssertInLoop()
{
    assert(std::this_thread::get_id() == _thread_id);
}

void EventLoop::QueueInLoop(const Functor &cb)
{
    // 保证最小粒度的加锁
    {
        std::unique_lock<std::mutex> _lock(_mtx);
        _tasks.push_back(cb);
    }
    // 唤醒可能因为没有事件就绪导致的epoll阻塞，就是在eventfd中写入数据，唤醒epoll
    WakeUpEventFd();
}

// 判断任务是否在当前线程中，是就执行，不是就插入队列
void EventLoop::RunInLoop(const Functor &cb)
{
    if (IsInLoop())
    {
        return cb();
    }
    return QueueInLoop(cb);
}

void EventLoop::UpdateEvents(Channel* channel)
{
    _poll.UpdateEvents(channel);
}

void EventLoop::RemoveEvent(Channel* channel)
{
    _poll.RemoveEvent(channel);
}

void EventLoop::Start()
{
    _isrunning = true;
    while (_isrunning)
    {
        // 1. 进行事件监控
        std::vector<Channel *> active;
        _poll.Poll(active);

        // 2. 事件处理
        for (auto &channel : active)
        {
            channel->HandleEvents();
        }

        // 3. 执行任务
        RunAllTask();
    }
}

// private
int EventLoop::CreateEventfd()
{
    int efd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (efd < 0)
    {
        if (errno == EINTR)
            return -1;
        ERR_LOG("eventfd error!");
        exit(-1);
    }
    return efd;
}

void EventLoop::SetEventCb()
{
    uint64_t val;
    int ret = read(_eventfd, &val, sizeof(val));
    if (ret < 0)
    {
        if (errno == EWOULDBLOCK || errno == EINTR)
            return;
        ERR_LOG("write error!");
        exit(-1);
    }
}

void EventLoop::WakeUpEventFd()
{
    uint64_t num = 1;
    int ret = write(_eventfd, &num, sizeof(num));
    if (ret < 0)
    {
        if (errno == EWOULDBLOCK || errno == EINTR)
            return;
        ERR_LOG("write error!");
        exit(-1);
    }
}

// 在时间轮中对任务操作
void EventLoop::AddTimer(uint64_t id, uint32_t timeout, task_t task)
{
    _wheel.AddTimerInLoop(id, timeout, task);
}

void EventLoop::RefreshTimer(uint64_t id)
{
    _wheel.RefreshTimerInLoop(id);
}

void EventLoop::CancelTimer(uint64_t id)
{
    _wheel.SetTimerCancelInLoop(id);
}

bool EventLoop::HasTimer(uint64_t id)
{
    return _wheel.HasTimerId(id);
}

////////////////////////////////ThreadLoop模块//////////////////////////////////////
/// 当线程来的时候，要创建Eventloop，_loop是临界资源，最好不要在锁外访问，否则会有竞态条件
void ThreadLoop::ThreadEntry()
{
    EventLoop loop;
    {
        std::unique_lock<std::mutex> _lock(_mutex);
        _loop = &loop;
    }
    _cond.notify_all();
    loop.Start();
}

ThreadLoop::ThreadLoop()
        : _loop(nullptr)
        , _thread(&ThreadLoop::ThreadEntry, this)
    {}

EventLoop* ThreadLoop::GetEventLoopPtr()
{
    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> _lock(_mutex);
        _cond.wait(_lock, [&]()mutable { return _loop != nullptr; });
        loop = _loop;
    }
    return loop;
}

