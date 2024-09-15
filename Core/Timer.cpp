//
// Created by ghost-him on 8/7/24.
//

#include "Timer.h"

void Handler::create_timer(uint64_t id,
                           const std::chrono::high_resolution_clock::time_point& createTime,
                           std::chrono::milliseconds _wait_time,
                           bool isPeriodic,
                           std::function<void()> callbackFunction) {
    this->id = id;
    this->_wait_time = _wait_time;
    this->_callback_function = std::move(callbackFunction);
    this->_is_periodic = isPeriodic;

    this->_last_time = createTime;
    std::chrono::milliseconds milliseconds {_wait_time + std::chrono::milliseconds (1)};
    this->_next_time = _last_time + milliseconds;
}

void Handler::update_timer() {
    this->_last_time = this->_next_time;
    this->_next_time = _last_time + _wait_time;
}


uint64_t Timer::set_timeout_timer(std::function<void()> func, std::chrono::milliseconds wait_time) {
    return create_timer(func, wait_time, false);
}

uint64_t Timer::set_periodic_timer(std::function<void()> func, std::chrono::milliseconds wait_time) {
    return create_timer(func, wait_time, true);
}

void Timer::remove_timer(uint64_t id) {
    std::lock_guard<std::mutex> guard{_timer_lock};
    _removed_timer_ID.insert(id);
}

void Timer::run() {
    while(_running) {
        std::lock_guard<std::mutex> guard{_timer_lock};
        auto nowTime {std::chrono::high_resolution_clock::now()};
        if (!_timers.empty()) {
            while(!_timers.empty() && nowTime >= _timers.top()._next_time) {
                auto timer {_timers.top()};
                _timers.pop();

                if (_removed_timer_ID.contains(timer.id)) {
                    continue;
                }

                _executor(timer._callback_function);
                if (timer._is_periodic) {
                    timer.update_timer();
                    _timers.push(timer);
                }
            }
        }
    }
}

uint64_t Timer::get_timer_ID() {
    return _nextID ++;
}

uint64_t Timer::create_timer(std::function<void()> func, std::chrono::milliseconds wait_time, bool is_periodic) {
    Handler handler;
    uint64_t id = get_timer_ID();
    handler.create_timer(id,
                         std::chrono::high_resolution_clock::now(),
                         wait_time,
                         is_periodic,
                         func);
    std::lock_guard<std::mutex> guard{_timer_lock};
    _timers.push(std::move(handler));
    return id;
}

void Timer::set_executor(std::function<void(std::function<void()>)> executor) {
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
    _running = true;
}

void Timer::stop() {
    _running = false;
}

