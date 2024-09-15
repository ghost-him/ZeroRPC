//
// Created by ghost-him on 8/8/24.
//

#include "Compression.h"
#include <brotli/encode.h>
#include <brotli/decode.h>
#include <stdexcept>

std::vector<std::byte> Brotli::compress(const std::span<const std::byte> &data) {
    size_t max_compressed_size = BrotliEncoderMaxCompressedSize(data.size());
    std::vector<std::byte> compressed(max_compressed_size);
    size_t encoded_size = max_compressed_size;

    BROTLI_BOOL result = BrotliEncoderCompress(
            BROTLI_DEFAULT_QUALITY,
            BROTLI_DEFAULT_WINDOW,
            BROTLI_DEFAULT_MODE,
            data.size(),
            reinterpret_cast<const uint8_t*>(data.data()),
            &encoded_size,
            reinterpret_cast<uint8_t*>(compressed.data()));

    if (result == BROTLI_FALSE) {
        throw std::runtime_error("Brotli compression failed");
    }

    compressed.resize(encoded_size);
    return compressed;
}

std::vector<std::byte> Brotli::decompress(const std::span<const std::byte> &compressed_data) {
    size_t decoded_size = 0;
    std::vector<std::byte> decompressed;

    BrotliDecoderState* state = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
    if (state == nullptr) {
        throw std::runtime_error("Failed to create Brotli decoder instance");
    }

    BrotliDecoderResult result;
    const uint8_t* next_in = reinterpret_cast<const uint8_t*>(compressed_data.data());
    size_t available_in = compressed_data.size();

    do {
        size_t available_out = decompressed.size() - decoded_size;
        uint8_t* next_out = reinterpret_cast<uint8_t*>(decompressed.data()) + decoded_size;

        result = BrotliDecoderDecompressStream(state, &available_in, &next_in,
                                               &available_out, &next_out, &decoded_size);

        if (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
            size_t new_size = decompressed.size() * 2;
            if (new_size == 0) new_size = 1024;
            decompressed.resize(new_size);
        }
    } while (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT);

    BrotliDecoderDestroyInstance(state);

    if (result != BROTLI_DECODER_RESULT_SUCCESS) {
        throw std::runtime_error("Brotli decompression failed");
    }

    decompressed.resize(decoded_size);
    return decompressed;
}
