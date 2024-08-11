//
// Created by ghost-him on 8/11/24.
//

#pragma once
#include "../Core/TcpClient.h"
#include "RpcPackage.h"
#include <future>
#include "../Core/ThreadPool.h"
#include "../Common/IDGenertor.hpp"


class RpcClient {
public:
    RpcClient(std::string_view host, uint16_t port);

    template<typename... Args>
    json call(const std::string& method, Args&&... args);

    void run();

private:
    ThreadPool* _thread_pool;
    TcpClient _tcpClient;

    std::mutex _hash_lock;
    std::unordered_map<std::string, std::promise<json>> _pending_request;

};

template<typename... Args>
json RpcClient::call(const std::string &method, Args&&... args) {
    RPCRequest request;
    request.method = method;
    request.params = json::array({std::forward<Args>(args)...});
    request.id = generate_uuid();

    json json_request = request;

    std::promise<json> response_promise;
    auto future = response_promise.get_future();
    {
        std::unique_lock<std::mutex> guard{_hash_lock};
        _pending_request[request.id] = std::move(response_promise);
    }

    _tcpClient.sendMessage(json_request.dump());
    return future.get();
}