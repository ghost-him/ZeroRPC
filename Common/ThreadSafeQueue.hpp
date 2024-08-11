//
// Created by ghost-him on 8/9/24.
//
#pragma once
#include <queue>
#include <mutex>

template <typename T>
class ThreadSafeQueue{
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
    std::queue<T> queue;
    std::mutex lock;
};

template<typename T>
T ThreadSafeQueue<T>::front_pop() {
    std::unique_lock<std::mutex> guard{lock};
    T res = queue.front();
    queue.pop();
    return res;
}

template<typename T>
bool ThreadSafeQueue<T>::empty() {
    std::unique_lock<std::mutex> guard{lock};
    return queue.empty();
}

template<typename T>
auto ThreadSafeQueue<T>::size() {
    std::unique_lock<std::mutex> guard{lock};
    return queue.size();
}

template<typename T>
void ThreadSafeQueue<T>::clear() {
    std::unique_lock<std::mutex> guard{lock};
    while(!queue.empty()) {
        queue.pop();
    }
}

template<typename T>
void ThreadSafeQueue<T>::pop() {
    std::unique_lock<std::mutex> guard{lock};
    queue.pop();
}

template<typename T>
void ThreadSafeQueue<T>::push(T &&data) {
    std::unique_lock<std::mutex> guard{lock};
    queue.push(std::move(data));
}

template<typename T>
void ThreadSafeQueue<T>::push(const T &data) {
    std::unique_lock<std::mutex> guard{lock};
    queue.push(data);
}

template<typename T>
const T& ThreadSafeQueue<T>::front() {
    std::unique_lock<std::mutex> guard{lock};
    return queue.front();
}


