#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class AsyncQueue
{
public:
    void push(const T &data)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(data);
        _cond.notify_one();
    }

    T pop()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while (_queue.empty())
        {
            _cond.wait(lock);
        }

        T top = _queue.front();
        _queue.pop();
        return top;
    }

private:
    std::queue<T> _queue;
    std::mutex _mutex;
    std::condition_variable _cond;
};