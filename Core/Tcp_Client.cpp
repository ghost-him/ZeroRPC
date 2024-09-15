//
// Created by ghost-him on 8/10/24.
//

#include "Tcp_Client.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory>
#include <iostream>

Tcp_Client::Tcp_Client(std::string_view addr, uint16_t port)
        : _ip(addr), _port(port), _running(false)
{
    set_send_message_callback([this](SocketChannelPtr channel){
        std::unique_lock<std::mutex> sendGuard{channel->_send_lock};
        if (!this->_running) {
            throw std::runtime_error("Client is not connected");
        }

        while(channel->_send_messages.size()) {
            auto message = channel->_send_messages.front_pop();
            ssize_t bytes_sent = ::send(_fd, message->data(), message->size(), MSG_NOSIGNAL);
            if (bytes_sent == -1) {
                throw std::system_error(errno, std::generic_category(), "Failed to send data");
            }
        }

    });
}

void Tcp_Client::run() {
    _fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (_fd == -1) {
        throw std::system_error(errno, std::generic_category(), "Failed to create socket");
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(_port);
    if (inet_pton(AF_INET, _ip.data(), &server_addr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid address");
    }

    int result = ::connect(_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
    if (result == -1 && errno != EINPROGRESS) {
        throw std::system_error(errno, std::generic_category(), "Failed to connect");
    }

    // Set TCP_NODELAY option
    int flag = 1;
    setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));

    _running = true;
    _channel = std::make_shared<Socket_Channel>(_fd, dynamic_cast<Network*>(this));
    _channel->set_compress_algo(_compressionType);
    _executor([this](){
        this->read_data();
    });
}

void Tcp_Client::disconnect() {
    if (_fd != -1) {
        close(_fd);
        _fd = -1;
    }
    _running = false;
}

void Tcp_Client::send_message(std::string_view data) {
    std::span<const std::byte> byte_span(
            reinterpret_cast<const std::byte*>(data.data()),
            data.size()
    );
    _channel->write_data(byte_span);
}

void Tcp_Client::read_data() {
    std::array<std::byte, BUFFER_SIZE> buffer;
    while(true) {
        if (!_running) {
            break;
        }
        ssize_t n = read(_channel->_fd, buffer.data(), BUFFER_SIZE);
        if (n == 0) {
            // 连接关闭
            disconnect();
            break;
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            // 发生错误
            disconnect();
            break;
        }
        if (n > 0) {
            _channel->_read_buffer.enqueue(buffer.begin(), buffer.begin() + n);
            this->_channel->parse_message();
        }
    }
}

void Tcp_Client::stop() {
    _running = false;
}
