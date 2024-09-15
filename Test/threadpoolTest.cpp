
#include "../Core/Thread_Pool.h"
#include "../Maintain/Thread_Pool_Monitor.hpp"
#include "gtest/gtest.h"

TEST(ThreadPoolTest, success_run) {
    Thread_Pool& pool = THREADPOOL;
    Thread_Pool_Monitor monitor(&pool);
    int count = 0;
    for (int i = 0; i < 32; i ++) {
        pool.commit([&](){
            std::cout << "当前的线程为第" + std::to_string(count ++) << "个" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(12));
        }, true);
    }
    // 以上操作应该在5s以内完成
    auto coreNum = std::thread::hardware_concurrency();
    ASSERT_EQ(monitor.get_working_thread_num(), coreNum);
    std::this_thread::sleep_for(std::chrono::seconds(6));
    // 线程扩充
    ASSERT_LT(coreNum, monitor.get_thread_num());
    std::this_thread::sleep_for(std::chrono::seconds(30));
    // 线程回收
    ASSERT_EQ(monitor.get_thread_num(), coreNum);

}


