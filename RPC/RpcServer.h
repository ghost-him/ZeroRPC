//
// Created by ghost-him on 8/11/24.
//
#pragma once

#include "../Core/TcpServer.h"
#include "../Common/Singleton.hpp"
#include "../Core/ThreadPool.h"
#include <unordered_map>
#include "RpcPackage.h"
#include "FunctionTraits.hpp"

class RpcServer {
public:
    RpcServer(int port);

    template<typename Func>
    void resigterMethod(std::string_view name, Func&& method);

    void run();

private:
    ThreadPool* _threadPool;
    std::unordered_map<std::string, RpcMethod> _methods;
    TcpServer _server;

    template<typename Func, typename... Args, std::size_t... I>
    static json invoke_helper(Func&& f, const json& params, std::index_sequence<I...>) {
        return json(std::invoke(std::forward<Func>(f), params[I].get<std::decay_t<Args>>()...));
    }

    template<typename Func, typename Indices = std::make_index_sequence<function_traits<std::decay_t<Func>>::arity>>
    static json invoke(Func&& f, const json& params) {
        return invoke_impl(std::forward<Func>(f), params, Indices{});
    }

    template<typename Func, std::size_t... I>
    static json invoke_impl(Func&& f, const json& params, std::index_sequence<I...>) {
        using traits = function_traits<std::decay_t<Func>>;
        return invoke_helper<Func, typename traits::template argument<I>::type...>
                (std::forward<Func>(f), params, std::index_sequence<I...>{});
    }



};

template<typename Func>
void RpcServer::resigterMethod(std::string_view name, Func&& method) {
    _methods[std::string(name)] = [this, method = std::forward<Func>(method)](const json& params) {
        return invoke(method, params);
    };
}

