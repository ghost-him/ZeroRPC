//
// Created by ghost-him on 8/8/24.
//

#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <thread>
#include <condition_variable>
#include <utility>
#include "../Common/Singleton.hpp"

#define THREADPOOL Singleton<Thread_Pool>::getInstance();

class Thread_Pool_Monitor;

class Thread_Pool {
    friend class Thread_Pool_Monitor;
public:
    using Task = std::function<void()>;
private:
    struct Handle {
        Task _task;
        bool _isIOTask { false };
        Handle() = default;
        Handle(Handle&& other) noexcept {
            _task = std::exchange(other._task, nullptr);
            _isIOTask = std::exchange(other._isIOTask, false);
        }
        ~Handle() = default;
    };
public:
    Thread_Pool();
    Thread_Pool(uint32_t minThread, uint32_t maxThread, uint32_t batchSize);

    void commit(const Task& func, bool isIOTask);

    void stop();

    ~Thread_Pool();
private:
    void init_thread_pool();

    void daemon_task();

    void work_task();

    void expend_thread_pool(uint32_t expendNum);
    void reduce_thread_pool(uint32_t reduceNum);

    const double expend_standard_ratio = 0.8;
    const double reduce_standard_ratio = 0.2;
private:
    bool _isStop;
    uint32_t _minThread;            // 最小的线程数
    uint32_t _maxThread;            // 最大的线程数
    uint32_t _batchSize;            // 每次增加的数量
    uint32_t _threadNum;            // 总线程数
    uint32_t _workingThreadNum;     // 正在工作的数量
    uint32_t _ioTaskNum;            // 处理io工作的数量
    uint32_t _hardwareCore;         // 硬件的核心数
    uint32_t _destroyedThreadNum;   // 要被清除的线程数

    std::condition_variable _taskCV;
    std::queue<Handle> _task_queue;
    std::unordered_map<std::thread::id, std::thread> _threads;
    std::thread _daemon_thread;
    std::mutex _pool_lock;

};