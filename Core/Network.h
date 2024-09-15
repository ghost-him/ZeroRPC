//
// Created by ghost-him on 8/10/24.
//

#pragma once

#include "Channel.h"
#include <unordered_set>


class Network {
    friend class Socket_Channel;
public:
    Network() = default;
    virtual void run() = 0;
    /*
     * 为客户端与服务端设置线程池
     */
    virtual void set_executor(std::function<void(std::function<void()>)> executor) {
        this->_executor = std::move(executor);
    }
    /*
     * 当收到一个完整的消息时会调用该函数
     */
    void set_read_message_callback(std::function<void(SocketChannelPtr ptr, DataPtr data)> message_handler) {
        this->_on_read_message = std::move(message_handler);
    }

    void set_send_message_callback(std::function<void(SocketChannelPtr ptr)> message_handler) {
        this->_on_send_message = std::move(message_handler);
    }

    void set_compress_algo(Compression_Type type) {
        this->_compressionType = type;
    }

    virtual ~Network() = default;

protected:
    Compression_Type _compressionType = Compression_Type::None;
    std::function<void(std::function<void()>)> _executor;
    std::function<void(SocketChannelPtr ptr, DataPtr data)> _on_read_message;
    std::function<void(SocketChannelPtr ptr)> _on_send_message;
};

