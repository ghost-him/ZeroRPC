//
// Created by ghost-him on 8/9/24.
//
#include "gtest/gtest.h"
#include "../Common/Buffer.hpp"
#include <deque>
#include <bitset>

TEST(BufferTest, init) {
    Common::Buffer<std::byte, 100> buffer;
    ASSERT_EQ(buffer.size(), 0);
}

TEST(BufferTest, add) {
    Common::Buffer<std::byte, 100> buffer;
    std::deque<std::byte> data;

    data.push_back(std::byte(5));

    buffer.enqueue(data.begin(), data.end());
    ASSERT_EQ(buffer.size(), 1);

    std::byte ans;
    buffer.dequeue(&ans, 1);
    ASSERT_EQ(ans, std::byte(5));
    ASSERT_EQ(buffer.size(), 0);

    data.clear();
    for (int i {0}; i < 10; i ++) {
        data.push_back(std::byte(i));
    }
    buffer.enqueue(data.begin(), data.end());
    ASSERT_EQ(buffer.size(), 10);



    std::array<std::byte, 10> out;
    buffer.dequeue(out.begin(), 10);
    ASSERT_EQ(buffer.size(), 0);

    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(data[i], out[i]);
        ASSERT_EQ(data[i], out.at(i));
    }

}

TEST(BufferTest, indexTest) {
    std::deque<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    Common::Buffer<int, 10> buffer;
    buffer.enqueue(data.begin(), data.end());

    for (int i = 0; i < 9; i ++) {
        ASSERT_EQ(i+1, buffer[i]);
        ASSERT_EQ(buffer[i], buffer.at(i));
    }
}

TEST(BufferTest, memConvertValue ) {
    int64_t value = 123456;
    auto res = variant2mem<int64_t>(value);


    for (int i = 0; i < sizeof(int64_t); i ++) {
        std::bitset<8> bits = std::to_integer<char>(res[i]);
        std::cout << bits;
    }
    std::cout << std::endl;

    auto ans = mem2variant<int64_t>(res);
    ASSERT_EQ(value, ans);
}