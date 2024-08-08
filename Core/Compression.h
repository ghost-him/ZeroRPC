//
// Created by ghost-him on 8/8/24.
//

#pragma once
#include <vector>
#include <cstddef>

class Compression {
public:
    virtual ~Compression() = default;
    virtual std::vector<std::byte> compress(const std::vector<std::byte>& data) = 0;
    virtual std::vector<std::byte> decompress(const std::vector<std::byte>& compressedData) = 0;
};

class Brotli : public Compression {
public:
    std::vector<std::byte> compress(const std::vector<std::byte>& data) override;
    std::vector<std::byte> decompress(const std::vector<std::byte>& compressedData) override;
};