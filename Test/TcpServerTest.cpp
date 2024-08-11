//
// Created by ghost-him on 8/9/24.
//
#include "../Core/TcpServer.h"
#include "gtest/gtest.h"
#include <stdexcept>
#include "../Core/ThreadPool.h"

TEST(ServerTest, run) {

    try {
        TcpServer ms(23333, 5);
        ms.setExecutor([](std::function<void()> function){
            ThreadPool& pool = THREADPOOL;
            pool.commit(function, true);
        });
        ms.setReadMessageCallback([](SocketChannelPtr channel, DataPtr ptr){
            std::cerr << "收到一个消息，并原路返回：";
            for (int i {0}; i < ptr->size(); i ++) {
                std::cerr << std::to_integer<char>((*ptr)[i]);
            }
            std::cerr << std::endl;
            std::span<std::byte> byte_span = std::span(ptr->data(), ptr->size());
            channel->writeData(byte_span);
        });

        ms.run();
    } catch (const std::exception& e) {
        std:: cerr << e.what() << std::endl;
    }

}
