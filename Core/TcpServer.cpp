//
// Created by ghost-him on 8/9/24.
//

#include "TcpServer.h"
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

void Reactor::addFd(int fd, uint32_t events) {
    epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        throw std::runtime_error("Failed to add _fd to epoll");
    }
}

void Reactor::removeFd(int fd) {
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

void Reactor::modifyFd(int fd, uint32_t events) {
    epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1) {
        throw std::runtime_error("Failed to modify _fd in epoll");
    }
}

TcpServer::TcpServer(int port, int num_workers): mainReactor(), stop(false) {
    listen_fd = createListenSocket(port);
    mainReactor.addFd(listen_fd, EPOLLIN | EPOLLET);

    subReactors.reserve(num_workers);
    for (int i = 0; i < num_workers; ++i) {
        subReactors.emplace_back(std::make_unique<Reactor>());
    }
    _executor = nullptr;

    setSendMessageCallback([this] (SocketChannelPtr channel){
        int fd = channel->fd;
        int reactorIndex = fdLoc[fd];
        auto& reactor = subReactors[reactorIndex];

        // 修改 epoll 事件,添加 EPOLLOUT
        reactor->modifyFd(fd, EPOLLIN | EPOLLOUT | EPOLLET);

        // 尝试立即发送数据
        trySend(channel);
    });
}

void TcpServer::run()  {
    if (_executor == nullptr) {
        throw std::runtime_error("反应堆未设置线程池");
    }

    for (size_t i = 0; i < subReactors.size(); ++i) {
        _executor([this, id = i](){
            this->subReactorRun(id);
        });
    }

    while (!stop) {
        auto events = mainReactor.wait(-1);
        for (const auto& event : events) {
            if (event.data.fd == listen_fd) {
                handleNewConnections();
            }
        }
    }
}

int TcpServer::createListenSocket(int port) {
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

void TcpServer::handleNewConnections() {
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int clientFd = accept4(listen_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len, SOCK_NONBLOCK);
        if (clientFd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;  // No more connections to accept
            } else {
                throw std::runtime_error("Failed to accept connection");
            }
        }

        int idx = clientFd % subReactors.size();
        subReactors[idx]->addFd(clientFd, EPOLLIN | EPOLLET);
        fdLoc[clientFd] = idx;
        SocketChannelPtr newChannel = std::make_shared<SocketChannel>(clientFd, dynamic_cast<Network*>(this));
        channels[clientFd] = newChannel;
    }
}

void TcpServer::subReactorRun(int index) {
    while (!stop) {
        auto events = subReactors[index]->wait(100);
        for (const auto& event : events) {
            if (event.events & EPOLLIN) {
                _executor([this, fd = event.data.fd](){
                    this->readData(fd);
                });
            }
        }
    }
}

void TcpServer::readData(int fd) {
    SocketChannelPtr channel = channels[fd];

    std::array<std::byte, BUFFER_SIZE> buffer;
    while (true) {
        ssize_t n = read(fd, buffer.data(), BUFFER_SIZE);
        if (n == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;  // No more _data to read
            } else {
                closeConnection(fd);
                return;
            }
        } else if (n == 0) {
            closeConnection(fd);
            return;
        }
        channel->readBuffer.enqueue(buffer.begin(), buffer.begin() + n);
        channel->parseMessage();
    }

    channel->parseMessage();

}

void TcpServer::closeConnection(int fd) {
    int loc = fdLoc[fd];
    subReactors[loc]->removeFd(fd);
    fdLoc.erase(fd);
    channels.erase(fd);
    close(fd);
}

void TcpServer::trySend(SocketChannelPtr channel) {
    std::unique_lock<std::mutex> guard(channel->sendLock);
    int fd = channel->fd;
    while (!channel->sendMessages.empty()) {
        DataPtr message = channel->sendMessages.front();
        ssize_t sent = send(fd, message->data() + channel->sendOffset, message->size() - channel->sendOffset, MSG_NOSIGNAL);

        if (sent > 0) {
            channel->sendOffset += sent;
            if (channel->sendOffset == message->size()) {
                channel->sendMessages.pop();
                channel->sendOffset = 0;
            }
        } else if (sent == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 发送缓冲区已满,等待下一次 EPOLLOUT 事件
                return;
            } else {
                // 发生错误,关闭连接
                closeConnection(fd);
                return;
            }
        }
    }

    // 所有数据已发送完毕,移除 EPOLLOUT 事件
    int reactorIndex = fdLoc[fd];
    auto& reactor = subReactors[reactorIndex];
    reactor->modifyFd(fd, EPOLLIN | EPOLLET);
}

