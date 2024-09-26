//
// Created by ghost-him on 8/11/24.
//

#pragma once
#include "../Core/Tcp_Client.h"
#include "Rpc_Package.h"
#include "../Core/Timer.h"
#include <future>
#include "../Core/Thread_Pool.h"
#include "../Common/ID_Genertor.hpp"
#include "../Common/prg_cfg.hpp"
#include "../Core/Data_Stream.h"

struct Rpc_Status: public enable_serializable {
    int clientID;
    std::string token; // 用于身份验证，长度为16字节

    SERIALIZE(clientID, token)
};

class Rpc_Client {
public:
    Rpc_Client(std::string_view host, uint16_t port);

    ~Rpc_Client();

    template<typename Ret, typename... Args>
    Ret call(const std::string& method, Args&&... args);

    void run();

    void set_compress_algo(Compression_Type type);

    Thread_Pool& get_thread_pool();

    Timer& get_timer();

private:

    void heartbeat_signal();

    Compression_Type _compressionType {Compression_Type::None};
    Thread_Pool* _thread_pool;
    Tcp_Client _tcpClient;

    std::mutex _hash_lock;
    std::unordered_map<std::string, std::promise<Data_Stream>> _pending_request;

    Timer _timer;
    // 客户端的身份信息
    Rpc_Status _status;
};

template<typename Ret, typename... Args>
Ret Rpc_Client::call(const std::string &method, Args&&... args) {
    RPCRequest request;
    request.method = method;
    Data_Stream ds;
    if constexpr (sizeof...(Args) > 0) {
        auto tuple = std::forward_as_tuple(std::forward<Args>(args)...);
        constexpr auto size = sizeof...(Args);
        [&]<size_t... Is>(std::index_sequence<Is...>) {
            (ds.write_args(std::get<size - 1 - Is>(tuple)), ...);
        }(std::make_index_sequence<size>{});
    }
    request.params = ds;
    request.id = generate_uuid();

    Data_Stream request_buf;
    request_buf << request;

    std::promise<Data_Stream> response_promise;
    auto future = response_promise.get_future();
    {
        std::unique_lock<std::mutex> guard{_hash_lock};
        _pending_request[request.id] = std::move(response_promise);
    }

    _tcpClient.send_message({request_buf.data().data(), request_buf.data().size()});

    Data_Stream result_buf = future.get();

    if constexpr (!std::is_void_v<Ret>) {
        Ret result;
        result_buf >> result;
        return result;
    }
}
