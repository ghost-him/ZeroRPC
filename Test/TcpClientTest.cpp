//
// Created by ghost-him on 8/10/24.
//
#include "../Core/Tcp_Client.h"
#include "gtest/gtest.h"
#include <stdexcept>
#include "../Core/Thread_Pool.h"
#include "../Core/Timer.h"

TEST(ClientTest, run) {

    try {
        Thread_Pool& pool = THREADPOOL;

        Timer timer;
        timer.set_executor([](std::function<void()> function){
            Thread_Pool& pool = THREADPOOL;
            pool.commit(function, true);
        });

        Tcp_Client ms("127.0.0.1", 23333);

        ms.set_executor([](std::function<void()> function){
            Thread_Pool& pool = THREADPOOL;
            pool.commit(function, true);
        });

        ms.set_read_message_callback([](SocketChannelPtr channel, DataPtr ptr){
            std::cerr << "receive new message : ";
            for (const auto& byte : (*ptr)) {
                char c = static_cast<char>(byte);
                std::cout << c;
            }
            std::cout << std::endl;
        });
        ms.set_compress_algo(Compression_Type::Brotli);
        ms.run();

        timer.set_periodic_timer([&](){
            std::cerr << "send message!" << std::endl;
            ms.send_message("hello!");
        }, std::chrono::seconds(2));

        timer.run();

    } catch (const std::exception& e) {
        std:: cerr << e.what() << std::endl;
    }

}
