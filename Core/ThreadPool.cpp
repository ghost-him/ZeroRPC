//
// Created by ghost-him on 8/8/24.
//

#include "ThreadPool.h"
#include <iostream>

ThreadPool::ThreadPool()
        : ThreadPool(std::thread::hardware_concurrency(),
                     std::thread::hardware_concurrency()*5,
                     std::thread::hardware_concurrency())
{

}

void ThreadPool::daemonTask() {
    while(true) {
        if (_isStop) {
            break;
        }

        std::unique_lock<std::mutex> guard(poolLock);
        // 如果工作线程大于总线程的80%，并且非io线程不等于核心数，则扩充
        uint32_t cpuTaskNum = _workingThreadNum - _ioTaskNum;
        if (
                _workingThreadNum > static_cast<uint32_t>(_threadNum * 0.8) &&
                cpuTaskNum < _hardwareCore &&
                _threadNum + _batchSize <= _maxThread
                )
        {
            std::cout << "扩充线程" << std::endl;
            // 扩充线程池
            expendThreadPool(_batchSize);
        } else if (
                _workingThreadNum < static_cast<uint32_t>(_threadNum * 0.2) &&
                _threadNum - _batchSize >= _minThread
                )
        {
            std::cout << "减少线程" << std::endl;
            // 减小线程池
            reduceThreadPool(_batchSize);
        }

        guard.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

ThreadPool::ThreadPool(uint32_t minThread, uint32_t maxThread, uint32_t batchSize) {
    _isStop = false;
    _hardwareCore = std::thread::hardware_concurrency();

    _minThread = minThread;
    _maxThread = maxThread;
    _batchSize = batchSize;

    _destroyedThreadNum = 0;
    _threadNum = 0;
    _workingThreadNum = 0;
    _ioTaskNum = 0;

    initThreadPool();
}

void ThreadPool::expendThreadPool(uint32_t expendNum) {
    for (int i {0}; i < expendNum; i ++) {
        std::thread newThread {std::thread(&ThreadPool::workTask, this)};
        auto threadID = newThread.get_id();
        threads.emplace(threadID, std::move(newThread));
        _threadNum ++;
    }
}

void ThreadPool::reduceThreadPool(uint32_t reduceNum) {
    _destroyedThreadNum = reduceNum;
    for (int i {0}; i < reduceNum; i ++) {
        _taskCV.notify_one();
    }
}

void ThreadPool::initThreadPool() {
    daemonThread = std::thread(&ThreadPool::daemonTask, this);
    expendThreadPool(_minThread);
}

void ThreadPool::commit(const ThreadPool::Task &func, bool isIOTask) {
    Handle handle;
    handle._task = func;
    handle._isIOTask = isIOTask;

    std::unique_lock<std::mutex> guard{poolLock};
    taskQueue.push(std::move(handle));
    guard.unlock();

    _taskCV.notify_one();
}

void ThreadPool::stop() {
    _isStop = true;
}

ThreadPool::~ThreadPool() {
    stop();
    _taskCV.notify_all();

    daemonThread.join();
    for (auto& i : threads) {
        i.second.join();
    }

    while(!taskQueue.empty()) taskQueue.pop();
    threads.clear();


}

void ThreadPool::workTask() {
    while(true) {
        std::unique_lock<std::mutex> guard(poolLock);
        while(taskQueue.empty() && !_isStop) {
            _taskCV.wait(guard);
            if (_destroyedThreadNum > 0) {
                _destroyedThreadNum --;
                threads[std::this_thread::get_id()].detach();
                threads.erase(std::this_thread::get_id());
                _threadNum --;
                return;
            }
        }

        if (_isStop) {
            _threadNum --;
            return;
        }

        Handle task = std::move(taskQueue.front());
        taskQueue.pop();
        _workingThreadNum ++;
        if (task._isIOTask) {
            _ioTaskNum ++;
        }
        guard.unlock();

        task._task();

        guard.lock();
        if (task._isIOTask) {
            _ioTaskNum --;
        }
        _workingThreadNum--;
        guard.unlock();
    }
}
