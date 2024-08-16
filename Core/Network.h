//
// Created by ghost-him on 8/10/24.
//

#pragma once

#include "Channel.h"
#include <unordered_set>


class Network {
    friend class SocketChannel;
public:
    Network() = default;
    virtual void run() = 0;
    /*
     * 为客户端与服务端设置线程池
     */
    virtual void setExecutor(std::function<void(std::function<void()>)> executor) {
        this->_executor = std::move(executor);
    }
    /*
     * 当收到一个完整的消息时会调用该函数
     */
    void setReadMessageCallback(std::function<void(SocketChannelPtr ptr, DataPtr data)> messageHandler) {
        this->_onReadMessage = std::move(messageHandler);
    }

    void setSendMessageCallback(std::function<void(SocketChannelPtr ptr)> messageHandler) {
        this->_onSendMessage = std::move(messageHandler);
    }

    void set_compress_algo(CompressionType type) {
        this->_compressionType = type;
    }

    virtual ~Network() = default;

protected:
    CompressionType _compressionType = CompressionType::None;
    std::function<void(std::function<void()>)> _executor;
    std::function<void(SocketChannelPtr ptr, DataPtr data)> _onReadMessage;
    std::function<void(SocketChannelPtr ptr)> _onSendMessage;
};

