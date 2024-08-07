//
// Created by ghost-him on 8/7/24.
//
#include "../Core/Timer.h"
#include <iostream>
#include <thread>
#include "gtest/gtest.h"

void defaultTimerFunction (std::function<void()> func) {
    func();
    std::cout << "执行了一次"<< std::endl;
}

TEST(TimerTest, act1) {
    Timer timer;
    timer.setExecutor(defaultTimerFunction);

    auto id1 = timer.setPeriodicTimer([](){
        std::cout << "过了5秒" << std::endl;
        std::cout << "=================" << std::endl;
    }, std::chrono::seconds(5));

    auto id2 = timer.setPeriodicTimer([](){
        std::cout << "过了一秒" << std::endl;
    }, std::chrono::seconds(1));

    std::thread the([&](){
        std::this_thread::sleep_for(std::chrono::seconds(12));
        timer.removeTimer(id2);
    });
    the.detach();
    std::thread the1([]()
    {
        std::this_thread::sleep_for(std::chrono::seconds(20));
        exit(0);
    });
    the1.detach();
    while(true) {
        timer.run();
    }

}