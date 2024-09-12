//
// Created by ghost-him on 8/11/24.
//

#pragma once
#include "../Core/TcpClient.h"
#include "RpcPackage.h"
#include "../Core/Timer.h"
#include <future>
#include "../Core/ThreadPool.h"
#include "../Common/IDGenertor.hpp"
#include "../Common/prg_cfg.hpp"

struct RpcStatus: public enable_serializable {
    int clientID;
    std::string token; // 用于身份验证，长度为16字节

    SERIALIZE(clientID, token)
};

class RpcClient {
public:
    RpcClient(std::string_view host, uint16_t port);

    ~RpcClient();

    template<typename Ret, typename... Args>
    Ret call(const std::string& method, Args&&... args);

    void run();

    void set_compress_algo(CompressionType type);

private:

    void heartbeat_signal();

    CompressionType _compressionType {CompressionType::None};
    ThreadPool* _thread_pool;
    TcpClient _tcpClient;

    std::mutex _hash_lock;
    std::unordered_map<std::string, std::promise<DataStream>> _pending_request;

    Timer _timer;
    // 客户端的身份信息
    RpcStatus _status;
};

template<typename Ret, typename... Args>
Ret RpcClient::call(const std::string &method, Args&&... args) {
    RPCRequest request;
    request.method = method;
    DataStream ds;
    if constexpr (sizeof...(Args) > 0) {
        auto tuple = std::forward_as_tuple(std::forward<Args>(args)...);
        constexpr auto size = sizeof...(Args);
        [&]<size_t... Is>(std::index_sequence<Is...>) {
            (ds.write_args(std::get<size - 1 - Is>(tuple)), ...);
        }(std::make_index_sequence<size>{});
    }
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

    if constexpr (!std::is_void_v<Ret>) {
        Ret result;
        result_buf >> result;
        return result;
    }
}
