//
// Created by ghost-him on 8/8/24.
//

#include "gtest/gtest.h"
#include "../Core/Compression.h"

TEST(CompressionTest, encodeAndDecode) {
    Brotli al;
    std::vector<std::byte> a;
    for (int i = 0; i < 1000; i ++) {
        a.push_back(std::byte(i));
    }
    auto res = al.compress(a);
    std::cout << "raw size: " << a.size() << std::endl;
    std::cout << "compressed size: " << res.size() << std::endl;

    auto back = al.decompress(res);
    ASSERT_EQ(back, a);
}
