//
// Created by ghost-him on 8/7/24.
//
#pragma once

#include <functional>
#include <cstdint>
#include <chrono>
#include <queue>
#include <mutex>
#include <unordered_set>
#include <atomic>


struct Handler {
    uint64_t id {0};
    std::function<void()> _callback_function;
    std::chrono::high_resolution_clock::time_point _last_time;
    std::chrono::high_resolution_clock::time_point _next_time;
    std::chrono::milliseconds _wait_time {0};
    bool _is_periodic {false};

    void create_timer(uint64_t id,
                      const std::chrono::high_resolution_clock::time_point& createTime,
                      std::chrono::milliseconds waitTime,
                      bool isPeriodic,
                      std::function<void()> callbackFunction);

    void update_timer();

    bool operator<(const Handler& other) const {
        return _next_time > other._next_time;
    }
};

class Timer{
private:

public:
    Timer(std::function<void(std::function<void()>)> executor = nullptr);

    uint64_t set_timeout_timer(std::function<void()> func, std::chrono::milliseconds wait_time);

    uint64_t set_periodic_timer(std::function<void()> func, std::chrono::milliseconds wait_time);

    void set_executor(std::function<void(std::function<void()>)> executor);

    void remove_timer(uint64_t id);

    void run();

    void stop();
private:

    uint64_t create_timer(std::function<void()> func, std::chrono::milliseconds wait_time, bool is_periodic);

    inline uint64_t get_timer_ID();

    std::priority_queue<Handler> _timers;
    std::unordered_set<uint64_t> _removed_timer_ID;
    uint64_t _nextID {1};
    std::mutex _timer_lock;
    std::atomic<bool> _running;
    std::function<void(std::function<void()>)> _executor;
};