//
// Created by ghost-him on 8/8/24.
//
#pragma once
#include "../Core/ThreadPool.h"

class ThreadPoolMonitor {
public:
    ThreadPoolMonitor(ThreadPool* pool) {
        this->_pool = pool;
    }
    void monitorThreadPool(ThreadPool* pool) {
        this->_pool = pool;
    }

    bool isStop() const {
        return _pool->_isStop;
    }

    uint32_t getMinThread() {
        return _pool->_minThread;
    }
    uint32_t getMaxThread() {
        return _pool->_maxThread;
    }
    uint32_t getBatchSize() {
        return _pool->_batchSize;
    }
    uint32_t getThreadNum() {
        return _pool->_threadNum;
    }
    uint32_t getWorkingThreadNum() {
        return _pool->_workingThreadNum;
    }
    uint32_t getIoTaskNum() {
        return _pool->_ioTaskNum;
    }
    uint32_t getHardwareCore() {
        return _pool->_hardwareCore;
    }
    uint32_t getCpuTaskNum() {
        return _pool->_workingThreadNum - _pool->_ioTaskNum;
    }
private:
    ThreadPool* _pool {nullptr};
};