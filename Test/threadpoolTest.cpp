#pragma once

#include "../Core/ThreadPool.h"
#include "../Maintain/ThreadPoolMonitor.hpp"
#include "gtest/gtest.h"

TEST(ThreadPoolTest, success_run) {
    ThreadPool& pool = THREADPOOL;
    ThreadPoolMonitor monitor(&pool);
    int count = 0;
    for (int i = 0; i < 32; i ++) {
        pool.commit([&](){
            std::cout << "当前的线程为第" + std::to_string(count ++) << "个" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(12));
        }, true);
    }
    // 以上操作应该在5s以内完成
    auto coreNum = std::thread::hardware_concurrency();
    ASSERT_EQ(monitor.getWorkingThreadNum(), coreNum);
    std::this_thread::sleep_for(std::chrono::seconds(6));
    // 线程扩充
    ASSERT_LT(coreNum, monitor.getThreadNum());
    std::this_thread::sleep_for(std::chrono::seconds(30));
    // 线程回收
    ASSERT_EQ(monitor.getThreadNum(), coreNum);

}


