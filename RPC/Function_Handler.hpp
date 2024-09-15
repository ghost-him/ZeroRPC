//
// Created by ghost-him on 8/11/24.
//

#pragma once


#include <functional>
#include <tuple>
#include "../Core/Data_Stream.h"
#include <any>
#include <optional>

class Handler_Base {
public:
    virtual ~Handler_Base() = default;
    virtual Data_Stream call(Data_Stream inArgs) = 0;
};


template<typename Ret, typename... Args>
class Function_Handler : public Handler_Base {
public:
    Function_Handler(std::function<Ret(Args...)> func) : func(func) {}

    Data_Stream call(Data_Stream inArgs) override {
        auto args = inArgs.get_args<Args...>();
        Ret ret = std::apply(func, args);
        Data_Stream result;
        result << ret;
        return result;
    }

private:
    std::function<Ret(Args...)> func;
};

// 针对void返回类型的特化实现
template<typename... Args>
class Function_Handler<void, Args...> : public Handler_Base {
public:
    Function_Handler(std::function<void(Args...)> func) : func(func) {}

    Data_Stream call(Data_Stream inArgs) override {
        auto args = inArgs.get_args<Args...>();
        std::apply(func, args);  // 调用函数，但不处理返回值
        return {};  // 对于void类型，返回一个空的DataStream
    }

private:
    std::function<void(Args...)> func;
};

class Handler_Manager {
public:
    template<typename Ret, typename ... Args>
    void registerHandler(std::string_view name, std::function<Ret(Args...)> func) {
        handlers[std::move(std::string(name))] = std::make_any<std::shared_ptr<Handler_Base>>(
                std::make_shared<Function_Handler<Ret, Args...>>(func)
        );
    }

    std::optional<Data_Stream> call(const std::string& name, Data_Stream args) {
        auto it = handlers.find(name);
        if (it != handlers.end()) {
            auto handlerPtr = std::any_cast<std::shared_ptr<Handler_Base>>(it->second);
            return handlerPtr->call(args);
        }
        throw std::runtime_error("Handler not found");
    }
private:
    std::map<std::string, std::any> handlers;
};