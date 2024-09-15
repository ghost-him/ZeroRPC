//
// Created by ghost-him on 8/8/24.
//

#pragma once
#include <vector>
#include <cstddef>
#include <span>

enum class Compression_Type {
    None = 0,
    Brotli = 1
};

class Compression {
public:
    virtual ~Compression() = default;
    virtual std::vector<std::byte> compress(const std::span<const std::byte>& data) = 0;
    virtual std::vector<std::byte> decompress(const std::span<const std::byte>& compressed_data) = 0;
};

class Brotli : public Compression {
public:
    std::vector<std::byte> compress(const std::span<const std::byte>& data) override;
    std::vector<std::byte> decompress(const std::span<const std::byte>& compressed_data) override;
};