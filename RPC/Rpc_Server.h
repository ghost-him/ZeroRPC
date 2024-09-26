//
// Created by ghost-him on 8/11/24.
//
#pragma once

#include "../Core/Tcp_Server.h"
#include "../Common/Singleton.hpp"
#include "../Core/Thread_Pool.h"
#include "../Core/Timer.h"
#include <unordered_map>
#include "Rpc_Package.h"
#include "Function_Handler.hpp"
#include "../Common/prg_cfg.hpp"
#include "Rpc_User.h"
#include "Rpc_User_Manager.h"

class Rpc_Server {
public:
    Rpc_Server(int port);

    template<typename Func>
    void register_method(std::string_view name, Func func);

    void run();

    void set_compress_algo(Compression_Type type);

    Thread_Pool& get_thread_pool();
    Timer& get_timer();

private:
    void handle_heartbeat_signal();

    Compression_Type _compressionType {Compression_Type::None};
    Thread_Pool* _threadPool;
    Handler_Manager _manager;
    Tcp_Server _server;
    Timer _timer;
    Rpc_User_Manager _rpcUserManager;
};

template<typename Func>
void Rpc_Server::register_method(std::string_view name, Func func) {
    // 使用 std::function 包装所有的可调用对象
    using FunctionType = typename std::function<decltype(func)>;
    _manager.registerHandler(name, std::function(func));
}