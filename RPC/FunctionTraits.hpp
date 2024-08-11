//
// Created by ghost-him on 8/11/24.
//

#pragma once


#include <functional>
#include <tuple>

// 主模板
template<typename T>
struct function_traits : public function_traits<decltype(&T::operator())> {};

// 特化版本，用于普通函数指针
template<typename R, typename... Args>
struct function_traits<R(*)(Args...)> {
    static constexpr size_t arity = sizeof...(Args);
    using result_type = R;
    using args_tuple = std::tuple<Args...>;

    template <size_t N>
    struct argument {
        static_assert(N < arity, "Invalid argument index.");
        using type = typename std::tuple_element<N, args_tuple>::type;
    };
};

// 特化版本，用于成员函数指针
template<typename C, typename R, typename... Args>
struct function_traits<R(C::*)(Args...)> : public function_traits<R(*)(Args...)> {};

// 特化版本，用于 const 成员函数指针
template<typename C, typename R, typename... Args>
struct function_traits<R(C::*)(Args...) const> : public function_traits<R(*)(Args...)> {};

// 特化版本，用于引用
template<typename T>
struct function_traits<T&> : public function_traits<T> {};

// 特化版本，用于右值引用
template<typename T>
struct function_traits<T&&> : public function_traits<T> {};

// 特化版本，用于 std::function
template<typename R, typename... Args>
struct function_traits<std::function<R(Args...)>> : public function_traits<R(*)(Args...)> {};
