//
// Created by ghost-him on 8/9/24.
//
#include "../Core/Tcp_Server.h"
#include "gtest/gtest.h"
#include <stdexcept>
#include "../Core/Thread_Pool.h"

TEST(ServerTest, run) {

    try {
        Tcp_Server ms(23333, 5);
        ms.set_executor([](std::function<void()> function){
            Thread_Pool& pool = THREADPOOL;
            pool.commit(function, true);
        });
        ms.set_read_message_callback([](SocketChannelPtr channel, DataPtr ptr){
            std::cerr << "收到一个消息，并原路返回：";
            for (const auto& byte : (*ptr)) {
                char c = static_cast<char>(byte);
                std::cout << c;
            }
            std::cout << std::endl;
            std::span<std::byte> byte_span = std::span(ptr->data(), ptr->size());
            channel->write_data(byte_span);
        });
        ms.set_compress_algo(Compression_Type::Brotli);
        ms.run();
    } catch (const std::exception& e) {
        std:: cerr << e.what() << std::endl;
    }

}
