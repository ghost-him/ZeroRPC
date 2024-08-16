//
// Created by ghost-him on 8/15/24.
//
#include "gtest/gtest.h"
#include "../Core/Channel.h"
#include <iostream>

TEST(ChannelTest, test_en_decode) {
    // 如果要执行这个测试，需要去`Channel.h`的文件中，将encodeString函数与decodeString设置为public

    /*
    SocketChannel channel(0, nullptr);

    std::string plain_text = "三向比较运算符可以用于确定两个值的大小顺序，也被称为太空飞船操作符。使用单个表达式，它可以告诉一个值是否等于，小于或大于另一个值。";

    channel.set_compress_algo(CompressionType::Brotli);

    std::cout << "plain text size: " << plain_text.size() << std::endl;
    std::vector<std::byte> encoded_string = channel.encodeString({(const std::byte*)plain_text.data(), plain_text.size()});
    std::cout << "compressed size: " << encoded_string.size() << std::endl;
    std::vector<std::byte> decoded_string = channel.decodeString(encoded_string);

    ASSERT_EQ(plain_text.size(), decoded_string.size());

    for (int i = 0; i < plain_text.size(); i ++) {
        //std::cout << plain_text[i] << " " << std::to_integer<char>(decoded_string[i]) << std::endl;
        ASSERT_EQ(std::byte(plain_text[i]), decoded_string[i]);
    }
     */
}