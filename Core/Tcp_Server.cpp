//
// Created by ghost-him on 8/9/24.
//

#include "Tcp_Server.h"
#include <cerrno>
#include <cstring>

Reactor::Reactor() : epoll_fd(epoll_create1(0)) {
    if (epoll_fd == -1) {
        throw std::runtime_error("Failed to create epoll instance");
    }
}

Reactor::~Reactor() {
    close(epoll_fd);
}

void Reactor::add_fd(int fd, uint32_t events) {
    epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        throw std::runtime_error("Failed to add _fd to epoll");
    }
}

void Reactor::remove_fd(int fd) {
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
        throw std::runtime_error("Failed to remove _fd from epoll");
    }
}

std::span <epoll_event> Reactor::wait(int timeout) {
    int nfds = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, timeout);
    if (nfds == -1) {
        throw std::runtime_error("epoll_wait failed"  + std::string(strerror(errno)));
    }
    return std::span(events.data(), nfds);
}

void Reactor::modify_fd(int fd, uint32_t events) {
    epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1) {
        throw std::runtime_error("Failed to modify _fd in epoll");
    }
}

Tcp_Server::Tcp_Server(int port, int num_workers): _main_reactor(), _stop(false) {
    _listen_fd = create_listen_socket(port);
    _main_reactor.add_fd(_listen_fd, EPOLLIN | EPOLLET);

    _sub_reactors.reserve(num_workers);
    for (int i = 0; i < num_workers; ++i) {
        _sub_reactors.emplace_back(std::make_unique<Reactor>());
    }
    _executor = nullptr;

    set_send_message_callback([this] (SocketChannelPtr channel){
        int fd = channel->_fd;
        int reactorIndex = _fd_location[fd];
        auto& reactor = _sub_reactors[reactorIndex];

        // 修改 epoll 事件,添加 EPOLLOUT
        reactor->modify_fd(fd, EPOLLIN | EPOLLOUT | EPOLLET);

        // 尝试立即发送数据
        try_send(channel);
    });
}

void Tcp_Server::run()  {
    if (_executor == nullptr) {
        throw std::runtime_error("反应堆未设置线程池");
    }

    for (size_t i = 0; i < _sub_reactors.size(); ++i) {
        _executor([this, id = i](){
            this->sub_reactor_run(id);
        });
    }
    while (!_stop) {
        auto events = _main_reactor.wait(-1);
        for (const auto& event : events) {
            if (event.data.fd == _listen_fd) {
                handle_new_connections();
            }
        }
    }
}

int Tcp_Server::create_listen_socket(int port) {
    {
        int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (fd == -1) {
            throw std::runtime_error("Failed to create socket");
        }

        int opt = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            close(fd);
            throw std::runtime_error("Failed to set SO_REUSEADDR");
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port);

        if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
            close(fd);
            throw std::runtime_error("Failed to bind");
        }

        if (listen(fd, SOMAXCONN) == -1) {
            close(fd);
            throw std::runtime_error("Failed to listen");
        }

        return fd;
    }
}

void Tcp_Server::handle_new_connections() {
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int clientFd = accept4(_listen_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len, SOCK_NONBLOCK);
        if (clientFd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;  // No more connections to accept
            } else {
                throw std::runtime_error("Failed to accept connection");
            }
        }

        int idx = clientFd % _sub_reactors.size();
        _sub_reactors[idx]->add_fd(clientFd, EPOLLIN | EPOLLET);
        _fd_location[clientFd] = idx;
        SocketChannelPtr newChannel = std::make_shared<Socket_Channel>(clientFd, dynamic_cast<Network*>(this));
        newChannel->set_compress_algo(_compressionType);
        _channels[clientFd] = newChannel;

        if (_on_new_connection) {
            _on_new_connection(clientFd);
        }
    }
}

void Tcp_Server::sub_reactor_run(int index) {
    while (!_stop) {
        auto events = _sub_reactors[index]->wait(100);
        for (const auto& event : events) {
            if (event.events & EPOLLIN) {
                _executor([this, fd = event.data.fd](){
                    this->read_data(fd);
                });
            }
        }
    }
}

void Tcp_Server::read_data(int fd) {
    SocketChannelPtr channel = _channels[fd];

    std::array<std::byte, BUFFER_SIZE> buffer;
    while (true) {
        ssize_t n = read(fd, buffer.data(), BUFFER_SIZE);
        if (n == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;  // No more _data to read
            } else {
                close_connection(fd);
                return;
            }
        } else if (n == 0) {
            close_connection(fd);
            return;
        }
        channel->_read_buffer.enqueue(buffer.begin(), buffer.begin() + n);
        channel->parse_message();
    }

    channel->parse_message();

}

void Tcp_Server::close_connection(int fd) {
    int loc = _fd_location[fd];
    _sub_reactors[loc]->remove_fd(fd);
    _fd_location.erase(fd);
    _channels.erase(fd);
    close(fd);

    if (_on_disconnection) {
        _on_disconnection(fd);
    }
}

void Tcp_Server::try_send(SocketChannelPtr channel) {
    std::unique_lock<std::mutex> guard(channel->_send_lock);
    int fd = channel->_fd;
    while (!channel->_send_messages.empty()) {
        DataPtr message = channel->_send_messages.front();
        ssize_t sent = send(fd, message->data() + channel->_send_offset, message->size() - channel->_send_offset, MSG_NOSIGNAL);

        if (sent > 0) {
            channel->_send_offset += sent;
            if (channel->_send_offset == message->size()) {
                channel->_send_messages.pop();
                channel->_send_offset = 0;
            }
        } else if (sent == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 发送缓冲区已满,等待下一次 EPOLLOUT 事件
                return;
            } else {
                // 发生错误,关闭连接
                close_connection(fd);
                return;
            }
        }
    }

    // 所有数据已发送完毕,移除 EPOLLOUT 事件
    int reactorIndex = _fd_location[fd];
    auto& reactor = _sub_reactors[reactorIndex];
    reactor->modify_fd(fd, EPOLLIN | EPOLLET);
}

void Tcp_Server::set_new_connection_callback(std::function<void(int)> connection_callback) {
    this->_on_new_connection = std::move(connection_callback);
}

void Tcp_Server::set_disconnect_callback(std::function<void(int)> disconnect_callback) {
    this->_on_disconnection = std::move(disconnect_callback);
}
