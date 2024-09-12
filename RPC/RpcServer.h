//
// Created by ghost-him on 8/11/24.
//
#pragma once

#include "../Core/TcpServer.h"
#include "../Common/Singleton.hpp"
#include "../Core/ThreadPool.h"
#include "../Core/Timer.h"
#include <unordered_map>
#include "RpcPackage.h"
#include "FunctionHandler.hpp"
#include "../Common/prg_cfg.hpp"
#include "RpcUser.h"
#include "RpcUserManager.h"

class RpcServer {
public:
    RpcServer(int port);

    template<typename Func>
    void registerMethod(std::string_view name, Func func);

    void run();

    void set_compress_algo(CompressionType type);

private:
    void handle_heartbeat_signal();

    CompressionType _compressionType {CompressionType::None};
    ThreadPool* _threadPool;
    HandlerManager _manager;
    TcpServer _server;
    Timer _timer;
    RpcUserManager _rpcUserManager;
};

template<typename Func>
void RpcServer::registerMethod(std::string_view name, Func func) {
    // 使用 std::function 包装所有的可调用对象
    using FunctionType = typename std::function<decltype(func)>;
    _manager.registerHandler(name, std::function(func));
}