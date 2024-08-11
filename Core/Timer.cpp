//
// Created by ghost-him on 8/7/24.
//

#include "Timer.h"

void Handler::createTimer(uint64_t id,
                 const std::chrono::high_resolution_clock::time_point& createTime,
                 std::chrono::milliseconds waitTime,
                 bool isPeriodic,
                 std::function<void()> callbackFunction) {
    this->id = id;
    this->waitTime = waitTime;
    this->callbackFunction = std::move(callbackFunction);
    this->isPeriodic = isPeriodic;

    this->lastTime = createTime;
    std::chrono::milliseconds milliseconds {waitTime + std::chrono::milliseconds (1)};
    this->nextTime = lastTime + milliseconds;
}

void Handler::updateTimer() {
    this->lastTime = this->nextTime;
    this->nextTime = lastTime + waitTime;
}


uint64_t Timer::setTimeoutTimer(std::function<void()> func, std::chrono::milliseconds waitTime) {
    return createTimer(func, waitTime, false);
}

uint64_t Timer::setPeriodicTimer(std::function<void()> func, std::chrono::milliseconds waitTime) {
    return createTimer(func, waitTime, true);
}

void Timer::removeTimer(uint64_t id) {
    std::lock_guard<std::mutex> guard{_timerLock};
    _removedTimerID.insert(id);
}

void Timer::run() {
    std::lock_guard<std::mutex> guard{_timerLock};
    auto nowTime {std::chrono::high_resolution_clock::now()};
    if (!_timers.empty()) {
        while(!_timers.empty() && nowTime >= _timers.top().nextTime) {
            auto timer {_timers.top()};
            _timers.pop();

            if (_removedTimerID.contains(timer.id)) {
                continue;
            }

            _executor(timer.callbackFunction);
            if (timer.isPeriodic) {
                timer.updateTimer();
                _timers.push(timer);
            }
        }
    }

}

uint64_t Timer::getTimerID() {
    return _nextID ++;
}

uint64_t Timer::createTimer(std::function<void()> func, std::chrono::milliseconds waitTime, bool isPeriodic) {
    Handler handler;
    uint64_t id = getTimerID();
    handler.createTimer(id,
                        std::chrono::high_resolution_clock::now(),
                        waitTime,
                        isPeriodic,
                        func);
    std::lock_guard<std::mutex> guard{_timerLock};
    _timers.push(std::move(handler));
    return id;
}

void Timer::setExecutor(std::function<void(std::function<void()>)> executor) {
    this->_executor = std::move(executor);
}

Timer::Timer(std::function<void(std::function<void()>)> executor) {
    if (executor != nullptr) {
        this->_executor = std::move(executor);
        return ;
    }
    this->_executor = [](std::function<void()> func) -> void {
        func();
    };
}

