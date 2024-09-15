//
// Created by ghost-him on 8/9/24.
//
#pragma once
#include <queue>
#include <mutex>

template <typename T>
class Thread_Safe_Queue{
public:
    void push(const T & data);
    void push(T && data);
    void pop();
    void clear();
    auto size();
    bool empty();
    const T& front();
    T front_pop();
private:
    std::queue<T> _queue;
    std::mutex _lock;
};

template<typename T>
T Thread_Safe_Queue<T>::front_pop() {
    std::unique_lock<std::mutex> guard{_lock};
    T res = _queue.front();
    _queue.pop();
    return res;
}

template<typename T>
bool Thread_Safe_Queue<T>::empty() {
    std::unique_lock<std::mutex> guard{_lock};
    return _queue.empty();
}

template<typename T>
auto Thread_Safe_Queue<T>::size() {
    std::unique_lock<std::mutex> guard{_lock};
    return _queue.size();
}

template<typename T>
void Thread_Safe_Queue<T>::clear() {
    std::unique_lock<std::mutex> guard{_lock};
    while(!_queue.empty()) {
        _queue.pop();
    }
}

template<typename T>
void Thread_Safe_Queue<T>::pop() {
    std::unique_lock<std::mutex> guard{_lock};
    _queue.pop();
}

template<typename T>
void Thread_Safe_Queue<T>::push(T &&data) {
    std::unique_lock<std::mutex> guard{_lock};
    _queue.push(std::move(data));
}

template<typename T>
void Thread_Safe_Queue<T>::push(const T &data) {
    std::unique_lock<std::mutex> guard{_lock};
    _queue.push(data);
}

template<typename T>
const T& Thread_Safe_Queue<T>::front() {
    std::unique_lock<std::mutex> guard{_lock};
    return _queue.front();
}


