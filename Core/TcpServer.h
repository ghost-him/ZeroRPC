//
// Created by ghost-him on 8/9/24.
//
#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include <functional>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <span>
#include <optional>
#include "Channel.h"
#include "Network.h"

constexpr int MAX_EVENTS = 1024;

class TcpServer;

class Reactor {
public:
    Reactor();

    ~Reactor();

    void addFd(int fd, uint32_t events);

    void removeFd(int fd);

    void modifyFd(int fd, uint32_t events);

    std::span<epoll_event> wait(int timeout);

private:
    int epoll_fd;
    std::array<epoll_event, MAX_EVENTS> events;
};


class TcpServer : public Network {
    friend class SocketChannel;
public:
    TcpServer(int port, int num_workers);

    void run() override;

    virtual ~TcpServer() = default;

    void setNewConnectionCallback(std::function<void(int)> connectionCallback);

    void setDisconnectCallback(std::function<void(int)> disconnectCallback);

    void closeConnection(int fd);

private:

    static int createListenSocket(int port);

    void handleNewConnections();

    void subReactorRun(int index);

    void trySend(SocketChannelPtr channel);
    /*
     * 接收数据
     */
    void readData(int fd);

    Reactor mainReactor;
    std::vector<std::unique_ptr<Reactor>> subReactors;

    int listen_fd;
    std::atomic<bool> stop;

    std::unordered_map<int, SocketChannelPtr> channels;
    std::unordered_map<int, int> fdLoc;

    /*
     * 如果有一个新的用户创建时，则调用，用于传出当前用户的id
     */
    std::function<void(int)> onNewConnection;

    std::function<void(int)> onDisconnection;
};
