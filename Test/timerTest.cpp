//
// Created by ghost-him on 8/7/24.
//
#include "../Core/Timer.h"
#include <iostream>
#include <thread>
#include "gtest/gtest.h"

TEST(TimerTest, act1) {
    Timer timer;

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

TEST(TimerTest, testTimeoutAction) {
    Timer timer;
    timer.setTimeoutTimer([](){
        std::cerr << "过了1s" << std::endl;
    }, std::chrono::seconds(1));
    while(true) {
        timer.run();
    }
}

TEST(TimerTest, testPeriodicAction) {
    Timer timer;
    timer.setPeriodicTimer([](){
        std::cerr << "过了1s" << std::endl;
    }, std::chrono::seconds(1));
    while(true) {
        timer.run();
    }
}

TEST(TimerTest, handleTest) {
    Handler handler;

    std::function<void()> func = [](){
        std::cout << "测试函数" << std::endl;
    };

    handler.createTimer(1,
                        std::chrono::high_resolution_clock ::now(),
                        std::chrono::seconds(3),
                        false,
                        func);
}