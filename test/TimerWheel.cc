#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <stdint.h>
#include <algorithm>

#include <unistd.h>

using task_t = std::function<void()>;
using release_t = std::function<void()>;

class TimerTask
{
public:
    TimerTask(uint64_t id, uint32_t timeout, task_t task)
        : _id(id), _timeout(timeout), _task_cb(task)
    {
    }

    void SetRelease(release_t release)
    {
        _release = release;
    }

    uint32_t GetTimeOut()
    {
        return _timeout;
    }

    ~TimerTask()
    {
        std::cout << "Timer task delete" << std::endl;
        _task_cb();
        _release();
    }

private:
    uint64_t _id;
    uint32_t _timeout;
    task_t _task_cb;
    release_t _release;
};

class TimerWheel
{
private:
    void RemoveTimer(uint64_t id)
    {
        std::unordered_map<uint64_t, std::weak_ptr<TimerTask>>::iterator it = _timers.find(id);
        if (it != _timers.end())
        {
            _timers.erase(it);
        }
    }

public:
    TimerWheel(int capacity)
        : _capacity(capacity), _tick(0)
    {
        // resize会调用对应位置的构造函数
        _wheel.resize(capacity);
    }

    void AddTimer(uint64_t id, uint32_t timeout, task_t task)
    {
        std::shared_ptr<TimerTask> tsp = std::make_shared<TimerTask>(id, timeout, task);

        tsp->SetRelease(std::bind(&TimerWheel::RemoveTimer, this, id));

        size_t pos = (_tick + timeout) % _capacity;
        _wheel[pos].push_back(tsp);

        _timers[id] = std::weak_ptr<TimerTask>(tsp);
    }

    void RefreshTimer(uint64_t id)
    {
        std::unordered_map<uint64_t, std::weak_ptr<TimerTask>>::iterator it = _timers.find(id);
        if (it != _timers.end())
        {
            std::shared_ptr<TimerTask> tsp(_timers[id].lock());
            for (auto& slot : _wheel)
            {
                slot.erase(std::remove(slot.begin(), slot.end(), tsp), slot.end());
            }

            size_t pos = (_tick + it->second.lock()->GetTimeOut()) % _capacity;
            _wheel[pos].push_back(tsp);
        }
        return;
    }

    ~TimerWheel()
    {
    }

private:
    int _capacity;
    int _tick;
    std::vector<std::vector<std::shared_ptr<TimerTask>>> _wheel;
    std::shared_ptr<TimerTask> _tsp;
    std::unordered_map<uint64_t, std::weak_ptr<TimerTask>> _timers;
};

void Print()
{
    std::cout << "hello" << std::endl;
}

int main()
{
    TimerWheel wheel(100);
    wheel.AddTimer(10, 1, Print);

    sleep(10);
    return 0;
}