#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include <functional>
#include <thread>
#include <atomic>


class ThreadManager
{
    inline static std::atomic<int> _threadCount = 0;
    inline static int THREAD_COUNT_MAX = std::min(12, static_cast<int>(std::thread::hardware_concurrency()));

public:
    template <typename FunctionType, typename... Args>
    static void try_async(FunctionType&& f_, Args&&... args_)
    {
        auto threadCount {_threadCount.load(std::memory_order_relaxed)};
        while (threadCount < THREAD_COUNT_MAX)
        {
            if(_threadCount.compare_exchange_strong(threadCount, threadCount+1, std::memory_order_release, std::memory_order_relaxed))
            {
                std::thread t([&]()
                {
                    f_(std::forward<Args>(args_)...);
                    auto threadCount {_threadCount.load(std::memory_order_relaxed)};
                    while (!_threadCount.compare_exchange_strong(threadCount, threadCount-1, std::memory_order_release, std::memory_order_relaxed));
                });
                t.detach();
                return;
            }
        }

        f_(std::forward<Args>(args_)...);
    }
};

#endif
