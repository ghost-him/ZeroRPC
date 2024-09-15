//
// Created by ghost-him on 8/9/24.
//

#include "Channel.h"
#include "Tcp_Server.h"
#include <cstring>

Socket_Channel::Socket_Channel(int fd, Network* server)
: _fd{fd}, _server{server}, _read_buffer_length{0} {

}

bool Socket_Channel::write_data(std::span<const std::byte> data) {
    const auto& encoded_data = encode_string(data);
    DataPtr ptr = std::make_shared<std::vector<std::byte>>(encoded_data.size() + HEAD_LENGTH);
    auto memHead = variant2mem<int32_t>(encoded_data.size());
    memcpy(ptr->data(), memHead.data(), HEAD_LENGTH);
    memcpy(ptr->data() + HEAD_LENGTH, encoded_data.data(), encoded_data.size());

    _send_messages.push(ptr);
    _server->_on_send_message(shared_from_this());

    return true;
}

void Socket_Channel::parse_message() {
    while(1) {
        if (_read_buffer_length == 0) {
            if (_read_buffer.size() > HEAD_LENGTH) {
                //表明当前已经有一个用于存放长度的值
                std::array<std::byte, HEAD_LENGTH> temp;
                _read_buffer.dequeue(temp.begin(), HEAD_LENGTH);
                _read_buffer_length = mem2variant<int32_t>(temp);
            } else {
                break;
            }
        }

        if (_read_buffer.size() >= _read_buffer_length) {
            std::array<std::byte, BUFFER_SIZE> tempBuffer;
            auto data_len = _read_buffer_length;
            _read_buffer.dequeue(tempBuffer.begin(), data_len);
            _read_buffer_length = 0;

            // 后处理
            DataPtr newData = std::make_shared<std::vector<std::byte>>(
                    decode_string({tempBuffer.begin(), tempBuffer.begin() + data_len})
                    );
            _read_messages.push(std::move(newData));
        }
    }
    while (!_read_messages.empty()) {
        auto DataPtr = _read_messages.front_pop();
        if (_server->_on_read_message) {
            _server->_on_read_message(shared_from_this(), DataPtr);
        }
    }
}


std::vector<std::byte> Socket_Channel::encode_string(const std::span<const std::byte> &plain_string) {
    std::vector<std::byte> ret;
    Compression* alg;
    switch(_compressionType) {
        case Compression_Type::None: {
            ret.push_back(static_cast<std::byte>(Compression_Type::None));
            alg = nullptr;
            break;
        }
        case Compression_Type::Brotli: {
            ret.push_back(static_cast<std::byte>(Compression_Type::Brotli));
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

std::vector<std::byte> Socket_Channel::decode_string(const std::span<const std::byte> &encoded_string) {
    std::vector<std::byte> ret;
    Compression* alg;
    switch (static_cast<Compression_Type>(encoded_string[0])) {
        case Compression_Type::None: {
            alg = nullptr;
            break;
        }
        case Compression_Type::Brotli: {
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

void Socket_Channel::set_compress_algo(Compression_Type type) {
    this->_compressionType = type;
}

int Socket_Channel::get_fd() {
    return this->_fd;
}
