//
// Created by ghost-him on 8/9/24.
//
#pragma once

#include <span>
#include <vector>
#include <memory>
#include <optional>
#include "../Common/Thread_Safe_Queue.hpp"
#include "../Common/Buffer.hpp"
#include <functional>
#include "Compression.h"

constexpr int BUFFER_SIZE = 2 * 8 * 1024 + 8; // 一次最多可以传输 64 * 1024 = 64KB 的数据，缓冲区设置为其2倍
constexpr int HEAD_LENGTH = sizeof(int32_t);


class Network;
class Socket_Channel;

using SocketChannelPtr = std::shared_ptr<Socket_Channel>;
using DataPtr = std::shared_ptr<std::vector<std::byte>>;

class Socket_Channel : public std::enable_shared_from_this<Socket_Channel> {
    friend class Tcp_Server;
    friend class Tcp_Client;
public:
    // 初始化
    Socket_Channel(int fd, Network* server);
    // 写数据
    bool write_data(std::span<const std::byte> data);

    void set_compress_algo(Compression_Type type);

    int get_fd();

private:
    /*
     * 读到了一个新的数据，如果有数据内容，则返回其内容，否则无返回
     */
    void parse_message();

    std::vector<std::byte> encode_string(const std::span<const std::byte>& plain_string);

    std::vector<std::byte> decode_string(const std::span<const std::byte>& encoded_string);


private:
    int _fd;
    Compression_Type _compressionType = Compression_Type::None;
    Common::Buffer<std::byte, BUFFER_SIZE> _read_buffer;
    uint64_t _read_buffer_length;

    Thread_Safe_Queue<DataPtr> _read_messages;

    Thread_Safe_Queue<DataPtr> _send_messages;
    uint64_t _send_offset {0};

    std::mutex _send_lock;
    Network* _server;
};

