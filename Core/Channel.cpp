//
// Created by ghost-him on 8/9/24.
//

#include "Channel.h"
#include "TcpServer.h"
#include <cstring>

SocketChannel::SocketChannel(int fd, Network* server)
:fd{fd}, server{server}, readBufferLength{0} {

}

bool SocketChannel::writeData(std::span<const std::byte> data) {
    const auto& encoded_data = encodeString(data);
    DataPtr ptr = std::make_shared<std::vector<std::byte>>(encoded_data.size() + HEAD_LENGTH);
    auto memHead = variant2mem<int32_t>(encoded_data.size());
    memcpy(ptr->data(), memHead.data(), HEAD_LENGTH);
    memcpy(ptr->data() + HEAD_LENGTH, encoded_data.data(), encoded_data.size());

    sendMessages.push(ptr);
    server->_onSendMessage(shared_from_this());

    return true;
}

void SocketChannel::parseMessage() {
    while(1) {
        if (readBufferLength == 0) {
            if (readBuffer.size() > HEAD_LENGTH) {
                //表明当前已经有一个用于存放长度的值
                std::array<std::byte, HEAD_LENGTH> temp;
                readBuffer.dequeue(temp.begin(), HEAD_LENGTH);
                readBufferLength = mem2variant<int32_t>(temp, 1, BUFFER_SIZE / 2);
            } else {
                break;
            }
        }

        if (readBuffer.size() >= readBufferLength) {
            std::array<std::byte, BUFFER_SIZE> tempBuffer;
            auto data_len = readBufferLength;
            readBuffer.dequeue(tempBuffer.begin(), data_len);
            readBufferLength = 0;

            // 后处理
            DataPtr newData = std::make_shared<std::vector<std::byte>>(
                    decodeString({tempBuffer.begin(), tempBuffer.begin() + data_len})
                    );
            readMessages.push(std::move(newData));
        }
    }
    while (!readMessages.empty()) {
        auto DataPtr = readMessages.front_pop();
        if (server->_onReadMessage) {
            server->_onReadMessage(shared_from_this(), DataPtr);
        }
    }
}


std::vector<std::byte> SocketChannel::encodeString(const std::span<const std::byte> &plain_string) {
    std::vector<std::byte> ret;
    Compression* alg;
    switch(compressionType) {
        case CompressionType::None: {
            ret.push_back(static_cast<std::byte>(CompressionType::None));
            alg = nullptr;
            break;
        }
        case CompressionType::Brotli: {
            ret.push_back(static_cast<std::byte>(CompressionType::Brotli));
            alg = new Brotli();
            break;
        }
    }

    if (alg != nullptr) {
        const auto& encoded_string =alg->compress(plain_string);
        ret.resize(1 + encoded_string.size());
        std::memcpy(ret.data() + 1, encoded_string.data(), encoded_string.size());
    } else {
        ret.resize(1 + plain_string.size());
        std::memcpy(ret.data() + 1, plain_string.data(), plain_string.size());
    }
    if (alg != nullptr) {
        delete alg;
    }
    return ret;
}

std::vector<std::byte> SocketChannel::decodeString(const std::span<const std::byte> &encoded_string) {
    std::vector<std::byte> ret;
    Compression* alg;
    switch (static_cast<CompressionType>(encoded_string[0])) {
        case CompressionType::None: {
            alg = nullptr;
            break;
        }
        case CompressionType::Brotli: {
            alg = new Brotli;
            break;
        }

    }

    if (alg != nullptr) {
        const auto& decoded_message = alg->decompress({encoded_string.data() + 1, encoded_string.size() - 1});
        ret.resize(decoded_message.size());
        std::memcpy(ret.data(), decoded_message.data(), decoded_message.size());
    } else {
        auto message_size = encoded_string.size() - 1;
        ret.resize(message_size);
        std::memcpy(ret.data(), encoded_string.data() + 1, message_size);
    }

    if (alg != nullptr) {
        delete alg;
    }
    return ret;
}

void SocketChannel::set_compress_algo(CompressionType type) {
    this->compressionType = type;
}
