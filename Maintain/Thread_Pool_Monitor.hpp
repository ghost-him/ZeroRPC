//
// Created by ghost-him on 8/8/24.
//
#pragma once
#include "../Core/Thread_Pool.h"

class Thread_Pool_Monitor {
public:
    Thread_Pool_Monitor(Thread_Pool* pool) {
        this->_pool = pool;
    }
    void monitor_threadPool(Thread_Pool* pool) {
        this->_pool = pool;
    }

    bool is_stop() const {
        return _pool->_isStop;
    }

    uint32_t get_min_thread() {
        return _pool->_minThread;
    }
    uint32_t get_max_thread() {
        return _pool->_maxThread;
    }
    uint32_t get_batch_size() {
        return _pool->_batchSize;
    }
    uint32_t get_thread_num() {
        return _pool->_threadNum;
    }
    uint32_t get_working_thread_num() {
        return _pool->_workingThreadNum;
    }
    uint32_t get_io_task_num() {
        return _pool->_ioTaskNum;
    }
    uint32_t get_hardware_core() {
        return _pool->_hardwareCore;
    }
    uint32_t get_cpu_task_num() {
        return _pool->_workingThreadNum - _pool->_ioTaskNum;
    }
private:
    Thread_Pool* _pool {nullptr};
};