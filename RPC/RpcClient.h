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

    template<typename Ret, typename... Args>
    Ret call(const std::string& method, Args&&... args);

    void run();

    void set_compress_algo(CompressionType type);

private:
    CompressionType _compressionType {CompressionType::None};
    ThreadPool* _thread_pool;
    TcpClient _tcpClient;

    std::mutex _hash_lock;
    std::unordered_map<std::string, std::promise<DataStream>> _pending_request;

};

template<typename Ret, typename... Args>
Ret RpcClient::call(const std::string &method, Args&&... args) {
    RPCRequest request;
    request.method = method;
    DataStream ds;
    ds.write_args(args...);

    request.params = ds;
    request.id = generate_uuid();

    DataStream request_buf;
    request_buf << request;

    std::promise<DataStream> response_promise;
    auto future = response_promise.get_future();
    {
        std::unique_lock<std::mutex> guard{_hash_lock};
        _pending_request[request.id] = std::move(response_promise);
    }

    _tcpClient.sendMessage({request_buf.data().data(), request_buf.data().size()});

    DataStream result_buf = future.get();
    Ret result;
    result_buf >> result;
    return result;
}