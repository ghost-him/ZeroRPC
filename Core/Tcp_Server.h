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

class Tcp_Server;

class Reactor {
public:
    Reactor();

    ~Reactor();

    void add_fd(int fd, uint32_t events);

    void remove_fd(int fd);

    void modify_fd(int fd, uint32_t events);

    std::span<epoll_event> wait(int timeout);

private:
    int epoll_fd;
    std::array<epoll_event, MAX_EVENTS> events;
};


class Tcp_Server : public Network {
    friend class SocketChannel;
public:
    Tcp_Server(int port, int num_workers);

    void run() override;

    virtual ~Tcp_Server() = default;

    void set_new_connection_callback(std::function<void(int)> connection_callback);

    void set_disconnect_callback(std::function<void(int)> disconnect_callback);

    void close_connection(int fd);

private:

    static int create_listen_socket(int port);

    void handle_new_connections();

    void sub_reactor_run(int index);

    void try_send(SocketChannelPtr channel);
    /*
     * 接收数据
     */
    void read_data(int fd);

    Reactor _main_reactor;
    std::vector<std::unique_ptr<Reactor>> _sub_reactors;

    int _listen_fd;
    std::atomic<bool> _stop;

    std::unordered_map<int, SocketChannelPtr> _channels;
    std::unordered_map<int, int> _fd_location;

    /*
     * 如果有一个新的用户创建时，则调用，用于传出当前用户的id
     */
    std::function<void(int)> _on_new_connection;

    std::function<void(int)> _on_disconnection;
};
