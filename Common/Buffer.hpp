//
// Created by ghost-him on 8/9/24.
//

#pragma once

#include <array>
#include <cstring> // memcpy
#include <stdexcept>
#include <mutex>
#include <bit>
#include <algorithm>


namespace Common {
    template<typename T, std::size_t Capacity>
    class Buffer {
    public:
        Buffer() = default;

        // 批量入队
        template<std::input_iterator It>
        void enqueue(It begin, It end);

        // 批量出队
        template<std::output_iterator<T> It>
        void dequeue(It out, std::size_t count);

        // 使用 [] 运算符访问元素，不进行边界检查
        T& operator[](std::size_t index) noexcept;

        // 使用 at() 函数访问元素，进行边界检查
        T& at(std::size_t index);

        // 获取元素个数
        [[nodiscard]] std::size_t size() noexcept;

        // 检查队列是否为空
        [[nodiscard]] bool empty() noexcept;

        // 检查队列是否已满
        [[nodiscard]] bool full() noexcept;

        // 清空队列
        void clear() noexcept;
    private:
        std::mutex _lock;
        std::array<T, Capacity> _data;
        std::size_t _head = 0;
        std::size_t _tail = 0;
        std::size_t _size = 0;
    };

    template<typename T, std::size_t Capacity>
    T &Buffer<T, Capacity>::at(std::size_t index) {
        std::unique_lock<std::mutex> guard{_lock};
        if (index >= _size) {
            throw std::out_of_range("Index out of range");
        }
        return _data[(_head + index) % Capacity];
    }

    template<typename T, std::size_t Capacity>
    T &Buffer<T, Capacity>::operator[](std::size_t index) noexcept {
        std::unique_lock<std::mutex> guard{_lock};
        return _data[(_head + index) % Capacity];
    }


    template<typename T, std::size_t Capacity>
    void Common::Buffer<T, Capacity>::clear() noexcept {
        std::unique_lock<std::mutex> guard{_lock};
        _head = _tail = _size = 0;
    }

    template<typename T, std::size_t Capacity>
    bool Common::Buffer<T, Capacity>::full() noexcept {
        std::unique_lock<std::mutex> guard{_lock};
        return _size == Capacity;
    }

    template<typename T, std::size_t Capacity>
    bool Common::Buffer<T, Capacity>::empty() noexcept {
        std::unique_lock<std::mutex> guard{_lock};
        return _size == 0;
    }

    template<typename T, std::size_t Capacity>
    std::size_t Common::Buffer<T, Capacity>::size() noexcept {
        std::unique_lock<std::mutex> guard{_lock};
        return _size;
    }

    template<typename T, std::size_t Capacity>
    template<std::output_iterator<T> It>
    void Common::Buffer<T, Capacity>::dequeue(It out, std::size_t count) {
        std::unique_lock<std::mutex> guard{_lock};
        if (count > _size) {
            throw std::runtime_error("Not enough elements in queue");
        }

        if constexpr (std::is_trivially_copyable_v<T>) {
            // 使用 memcpy 优化 trivially copyable 类型
            std::size_t firstPart = std::min(Capacity - _head, count);
            std::memcpy(&(*out), &_data[_head], firstPart * sizeof(T));

            if (firstPart < count) {
                // 处理环绕情况
                std::memcpy(&(*(out + firstPart)), &_data[0], (count - firstPart) * sizeof(T));
            }
        } else {
            // 对于非 trivially copyable 类型，使用逐个复制
            for (std::size_t i = 0; i < count; ++i) {
                *out++ = _data[_head];
                _head = (_head + 1) % Capacity;
            }
        }

        _head = (_head + count) % Capacity;
        _size -= count;
    }

    template<typename T, std::size_t Capacity>
    template<std::input_iterator It>
    void Common::Buffer<T, Capacity>::enqueue(It begin, It end) {
        std::unique_lock<std::mutex> guard{_lock};
        std::size_t count = std::distance(begin, end);
        if (count > Capacity - _size) {
            throw std::runtime_error("Queue overflow");
        }

        if constexpr (std::is_trivially_copyable_v<T>) {
            // 使用 memcpy 优化 trivially copyable 类型
            std::size_t firstPart = std::min(Capacity - _tail, count);
            std::memcpy(&_data[_tail], &(*begin), firstPart * sizeof(T));

            if (firstPart < count) {
                // 处理环绕情况
                std::memcpy(&_data[0], &(*(begin + firstPart)), (count - firstPart) * sizeof(T));
            }
        } else {
            // 对于非 trivially copyable 类型，使用逐个复制
            for (auto it = begin; it != end; ++it) {
                _data[_tail] = *it;
                _tail = (_tail + 1) % Capacity;
            }
        }

        _tail = (_tail + count) % Capacity;
        _size += count;
    }
}

template<typename T>
struct type_identity {
    using type = T;
};


// 用默认的读法，如果值的范围不在规定的范围内，则用小端的方式
template<typename T>
T mem2variant(std::span<std::byte> memory) requires std::totally_ordered<T> {
    T res {};
    if (memory.size() < sizeof(T)) {
        throw std::runtime_error("mem2variant convert error! invalid memory size!");
    }
    if (std::endian::native == std::endian::big) {
        std::reverse(memory.begin(), memory.end());
        std::memcpy(&res, memory.data(), sizeof (T));
        std::reverse(memory.begin(), memory.end());
    } else {
        std::memcpy(&res, memory.data(), sizeof (T));
    }
    return res;
}

template<typename T>
std::array<std::byte, sizeof(T)> variant2mem(typename type_identity<T>::type value) requires std::totally_ordered<T> {
    std::array<std::byte, sizeof(T)> res;
    std::memcpy(res.data(), &value, sizeof(T));
    if (std::endian::native == std::endian::big) {
        std::reverse(res.begin(), res.end());
    }
    return res;
}
