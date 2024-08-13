//
// Created by ghost-him on 8/11/24.
//
#pragma once

#include "../Core/TcpServer.h"
#include "../Common/Singleton.hpp"
#include "../Core/ThreadPool.h"
#include <unordered_map>
#include "RpcPackage.h"
#include "FunctionHandler.hpp"

class RpcServer {
public:
    RpcServer(int port);

    template<typename Ret, typename... Args>
    void registerMethod(std::string_view name, Ret(*func)(Args...));

    void run();

private:
    ThreadPool* _threadPool;
    HandlerManager _manager;
    TcpServer _server;
};

template<typename Ret, typename... Args>
void RpcServer::registerMethod(std::string_view name, Ret(*func)(Args...)) {
    _manager.registerHandler(name, func);
}