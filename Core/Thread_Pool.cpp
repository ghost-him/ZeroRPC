//
// Created by ghost-him on 8/8/24.
//

#include "Thread_Pool.h"
#include <iostream>

Thread_Pool::Thread_Pool()
        : Thread_Pool(std::thread::hardware_concurrency(),
                     std::thread::hardware_concurrency()*5,
                      std::thread::hardware_concurrency())
{

}

void Thread_Pool::daemon_task() {
    while(true) {
        if (_isStop) {
            break;
        }

        std::unique_lock<std::mutex> guard(_pool_lock);
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
            expend_thread_pool(_batchSize);
        } else if (
                _workingThreadNum < static_cast<uint32_t>(_threadNum * 0.2) &&
                _threadNum - _batchSize >= _minThread
                )
        {
            std::cout << "减少线程" << std::endl;
            // 减小线程池
            reduce_thread_pool(_batchSize);
        }

        guard.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

Thread_Pool::Thread_Pool(uint32_t minThread, uint32_t maxThread, uint32_t batchSize) {
    _isStop = false;
    _hardwareCore = std::thread::hardware_concurrency();

    _minThread = minThread;
    _maxThread = maxThread;
    _batchSize = batchSize;

    _destroyedThreadNum = 0;
    _threadNum = 0;
    _workingThreadNum = 0;
    _ioTaskNum = 0;

    init_thread_pool();
}

void Thread_Pool::expend_thread_pool(uint32_t expendNum) {
    for (int i {0}; i < expendNum; i ++) {
        std::thread newThread {std::thread(&Thread_Pool::work_task, this)};
        auto threadID = newThread.get_id();
        _threads.emplace(threadID, std::move(newThread));
        _threadNum ++;
    }
}

void Thread_Pool::reduce_thread_pool(uint32_t reduceNum) {
    _destroyedThreadNum = reduceNum;
    for (int i {0}; i < reduceNum; i ++) {
        _taskCV.notify_one();
    }
}

void Thread_Pool::init_thread_pool() {
    _daemon_thread = std::thread(&Thread_Pool::daemon_task, this);
    expend_thread_pool(_minThread);
}

void Thread_Pool::commit(const Thread_Pool::Task &func, bool isIOTask) {
    Handle handle;
    handle._task = func;
    handle._isIOTask = isIOTask;

    std::unique_lock<std::mutex> guard{_pool_lock};
    _task_queue.push(std::move(handle));
    guard.unlock();

    _taskCV.notify_one();
}

void Thread_Pool::stop() {
    _isStop = true;
}

Thread_Pool::~Thread_Pool() {
    stop();
    _taskCV.notify_all();

    _daemon_thread.join();
    for (auto& i : _threads) {
        i.second.join();
    }

    while(!_task_queue.empty()) _task_queue.pop();
    _threads.clear();


}

void Thread_Pool::work_task() {
    while(true) {
        std::unique_lock<std::mutex> guard(_pool_lock);
        while(_task_queue.empty() && !_isStop) {
            _taskCV.wait(guard);
            if (_destroyedThreadNum > 0) {
                _destroyedThreadNum --;
                _threads[std::this_thread::get_id()].detach();
                _threads.erase(std::this_thread::get_id());
                _threadNum --;
                return;
            }
        }

        if (_isStop) {
            _threadNum --;
            return;
        }

        Handle task = std::move(_task_queue.front());
        _task_queue.pop();
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
