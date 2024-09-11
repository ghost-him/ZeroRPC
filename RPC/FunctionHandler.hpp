//
// Created by ghost-him on 8/11/24.
//

#pragma once


#include <functional>
#include <tuple>
#include "../Core/DataStream.h"
#include <any>

class HandlerBase {
public:
    virtual ~HandlerBase() = default;
    virtual DataStream call(DataStream inArgs) = 0;
};


template<typename Ret, typename... Args>
class FunctionHandler : public HandlerBase {
public:
    FunctionHandler(std::function<Ret(Args...)> func) : func(func) {}

    DataStream call(DataStream inArgs) override {
        auto args = inArgs.get_args<Args...>();
        Ret ret = std::apply(func, args);
        DataStream result;
        result << ret;
        return result;
    }

private:
    std::function<Ret(Args...)> func;
};

// 针对void返回类型的特化实现
template<typename... Args>
class FunctionHandler<void, Args...> : public HandlerBase {
public:
    FunctionHandler(std::function<void(Args...)> func) : func(func) {}

    DataStream call(DataStream inArgs) override {
        auto args = inArgs.get_args<Args...>();
        std::apply(func, args);  // 调用函数，但不处理返回值
        return {};  // 对于void类型，返回一个空的DataStream
    }

private:
    std::function<void(Args...)> func;
};

class HandlerManager {
public:
    template<typename Ret, typename ... Args>
    void registerHandler(std::string_view name, std::function<Ret(Args...)> func) {
        handlers[std::move(std::string(name))] = std::make_any<std::shared_ptr<HandlerBase>>(
                std::make_shared<FunctionHandler<Ret, Args...>>(func)
        );
    }

    std::optional<DataStream> call(const std::string& name, DataStream args) {
        auto it = handlers.find(name);
        if (it != handlers.end()) {
            auto handlerPtr = std::any_cast<std::shared_ptr<HandlerBase>>(it->second);
            return handlerPtr->call(args);
        }
        throw std::runtime_error("Handler not found");
    }
private:
    std::map<std::string, std::any> handlers;
};