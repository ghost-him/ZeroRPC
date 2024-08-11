//
// Created by ghost-him on 8/10/24.
//
#include "../Core/TcpClient.h"
#include "gtest/gtest.h"
#include <stdexcept>
#include "../Core/ThreadPool.h"
#include "../Core/Timer.h"

TEST(ClientTest, run) {

    try {
        ThreadPool& pool = THREADPOOL;

        Timer timer;
        timer.setExecutor([](std::function<void()> function){
            ThreadPool& pool = THREADPOOL;
            pool.commit(function, true);
        });

        TcpClient ms("127.0.0.1", 23333);

        ms.setExecutor([](std::function<void()> function){
            ThreadPool& pool = THREADPOOL;
            pool.commit(function, true);
        });

        ms.setReadMessageCallback([](SocketChannelPtr channel, DataPtr ptr){
            std::cerr << "receive new message : ";
            for (const auto& byte : (*ptr)) {
                char c = static_cast<char>(byte);
                if (std::isprint(c)) {
                    std::cout << c;
                } else {
                    std::cout << '.';
                }
            }
            std::cout << std::endl;
        });

        ms.run();

        timer.setPeriodicTimer([&](){
            std::cerr << "send message!" << std::endl;
            ms.sendMessage("hello!");
        }, std::chrono::seconds(2));
        while(1) {
            timer.run();
        }
    } catch (const std::exception& e) {
        std:: cerr << e.what() << std::endl;
    }

}
