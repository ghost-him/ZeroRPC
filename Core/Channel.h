//
// Created by ghost-him on 8/9/24.
//
#pragma once

#include <span>
#include <vector>
#include <memory>
#include <optional>
#include "../Common/ThreadSafeQueue.hpp"
#include "../Common/Buffer.hpp"
#include <functional>
#include "Compression.h"

constexpr int BUFFER_SIZE = 2 * 32 * 1024 + 8; // 一次最多可以传输 64 * 1024 = 64KB 的数据，缓冲区设置为其2倍
constexpr int HEAD_LENGTH = sizeof(int32_t);


class Network;
class SocketChannel;

using SocketChannelPtr = std::shared_ptr<SocketChannel>;
using DataPtr = std::shared_ptr<std::vector<std::byte>>;

class SocketChannel : public std::enable_shared_from_this<SocketChannel> {
    friend class TcpServer;
    friend class TcpClient;
public:
    // 初始化
    SocketChannel(int fd, Network* server);
    // 写数据
    bool writeData(std::span<const std::byte> data);

    void set_compress_algo(CompressionType type);

private:
    /*
     * 读到了一个新的数据，如果有数据内容，则返回其内容，否则无返回
     */
    void parseMessage();

    std::vector<std::byte> encodeString(const std::span<const std::byte>& plain_string);

    std::vector<std::byte> decodeString(const std::span<const std::byte>& encoded_string);


private:
    int fd;
    CompressionType compressionType = CompressionType::None;
    Common::Buffer<std::byte, BUFFER_SIZE> readBuffer;
    uint64_t readBufferLength;

    ThreadSafeQueue<DataPtr> readMessages;

    ThreadSafeQueue<DataPtr> sendMessages;
    uint64_t sendOffset {0};

    std::mutex sendLock;
    Network* server;
};

