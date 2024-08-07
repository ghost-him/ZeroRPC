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


class Timer{
private:
    struct Handler {
        uint64_t id {0};
        std::function<void()> callbackFunction;
        std::chrono::high_resolution_clock::time_point lastTime;
        std::chrono::high_resolution_clock::time_point nextTime;
        std::chrono::milliseconds waitTime {0};
        bool isPeriodic {false};

        void createTimer(uint64_t id,
                         const std::chrono::high_resolution_clock::time_point& createTime,
                         std::chrono::milliseconds waitTime,
                         bool isPeriodic,
                         std::function<void()> callbackFunction);

        void updateTimer();

        bool operator<(const Handler& other) const {
            return nextTime > other.nextTime;
        }
    };
public:

    uint64_t setTimeoutTimer(std::function<void()> func, std::chrono::milliseconds waitTime);

    uint64_t setPeriodicTimer(std::function<void()> func, std::chrono::milliseconds waitTime);

    void setExecutor(std::function<void(std::function<void()>)> executor);

    void removeTimer(uint64_t id);

    void run();
private:

    uint64_t createTimer(std::function<void()> func, std::chrono::milliseconds waitTime, bool isPeriodic);

    inline uint64_t getTimerID();

    std::priority_queue<Handler> _timers;
    std::unordered_set<uint64_t> _removedTimerID;
    uint64_t _nextID {1};
    std::mutex _timerLock;
    std::function<void(std::function<void()>)> _executor;
};