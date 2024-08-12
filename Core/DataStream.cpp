//
// Created by ghost-him on 8/12/24.
//

#include "DataStream.h"
#include <cstring>
#include <iostream>
#include <bit>
#include <algorithm>

void DataStream::write_memory(const char* data, uint32_t len) {
    reserve(len);
    auto size {_buf.size()};
    _buf.resize(size + len);

    std::memcpy(&_buf[size], data, len);
}

void DataStream::reserve(uint32_t len) {
    auto size {_buf.size()};
    auto cap {_buf.capacity()};

    if (size + len > cap) {
        while(size + len > cap) {
            if (cap == 0) {
                cap = 1;
            } else {
                cap *= 2;
            }
        }
        _buf.reserve(cap);
    }
}

void DataStream::write(bool value) {
    char type {DataType::BOOL};
    write_memory(&type, sizeof(char));
    write_memory(reinterpret_cast<char *>(&value), sizeof(bool));
}

void DataStream::write(char value) {
    char type {DataType::CHAR};
    write_memory(&type, sizeof(char));
    write_memory(reinterpret_cast<char *>(&value), sizeof(char));
}

void DataStream::write(int32_t value) {
    char type {DataType::INT32};
    write_memory(&type, sizeof(char));
    if (is_big_endian) {
        char * first = reinterpret_cast<char*>(&value);
        char * last = first + sizeof(int32_t);
        std::reverse(first, last);
    }
    write_memory(reinterpret_cast<char *>(&value), sizeof(int32_t));
}

void DataStream::write(int64_t value) {
    char type {DataType::INT64};
    write_memory(&type, sizeof(char));
    if (is_big_endian) {
        char * first = reinterpret_cast<char*>(&value);
        char * last = first + sizeof(int64_t);
        std::reverse(first, last);
    }
    write_memory(reinterpret_cast<char *>(&value), sizeof(int64_t));
}

void DataStream::write(float value) {
    char type {DataType::FLOAT};
    write_memory(&type, sizeof(char));
    if (is_big_endian) {
        char * first = reinterpret_cast<char*>(&value);
        char * last = first + sizeof(float);
        std::reverse(first, last);
    }
    write_memory(reinterpret_cast<char *>(&value), sizeof(float));
}

void DataStream::write(double value) {
    char type {DataType::DOUBLE};
    write_memory(&type, sizeof(char));
    if (is_big_endian) {
        char * first = reinterpret_cast<char*>(&value);
        char * last = first + sizeof(double);
        std::reverse(first, last);
    }
    write_memory(reinterpret_cast<char *>(&value), sizeof(double));
}

void DataStream::write(std::string_view data) {
    char type {DataType::STRING};
    write_memory(&type, sizeof(char));
    write(static_cast<int32_t>(data.size()));
    write_memory(data.data(), data.size());
}

bool DataStream::read(bool &value) {
    if (_buf[_pos] != DataType::BOOL) {
        return false;
    }
    ++_pos;
    value = _buf[_pos];
    ++_pos;
    return true;
}

bool DataStream::read(char &value) {
    if (_buf[_pos] != DataType::CHAR) {
        return false;
    }
    ++_pos;
    value = _buf[_pos];
    ++_pos;
    return true;
}

bool DataStream::read(int32_t &value) {
    if (_buf[_pos] != DataType::INT32) {
        return false;
    }
    ++_pos;
    value = *(reinterpret_cast<int32_t*>(&_buf[_pos]));
    if (is_big_endian) {
        char * first = reinterpret_cast<char*>(&value);
        char * last = first + sizeof(int32_t);
        std::reverse(first, last);
    }
    _pos += sizeof(int32_t);
    return true;
}

bool DataStream::read(int64_t &value) {
    if (_buf[_pos] != DataType::INT64) {
        return false;
    }
    ++_pos;
    value = *(reinterpret_cast<int64_t*>(&_buf[_pos]));
    if (is_big_endian) {
        char * first = reinterpret_cast<char*>(&value);
        char * last = first + sizeof(int64_t);
        std::reverse(first, last);
    }
    _pos += sizeof(int64_t);
    return true;
}

bool DataStream::read(float &value) {
    if (_buf[_pos] != DataType::FLOAT) {
        return false;
    }
    ++_pos;
    value = *(reinterpret_cast<float*>(&_buf[_pos]));
    if (is_big_endian) {
        char * first = reinterpret_cast<char*>(&value);
        char * last = first + sizeof(float);
        std::reverse(first, last);
    }
    _pos += sizeof(float);
    return true;
}

bool DataStream::read(double &value) {
    if (_buf[_pos] != DataType::DOUBLE) {
        return false;
    }
    ++_pos;
    value = *(reinterpret_cast<double*>(&_buf[_pos]));
    if (is_big_endian) {
        char * first = reinterpret_cast<char*>(&value);
        char * last = first + sizeof(double);
        std::reverse(first, last);
    }
    _pos += sizeof(double);
    return true;
}

bool DataStream::read(std::string &value) {
    if (_buf[_pos] != DataType::STRING) {
        return false;
    }
    ++_pos;
    int32_t len;
    read(len);
    if (len < 0) {
        return false;
    }
    value.assign(reinterpret_cast<char*>(&(_buf[_pos])), len);
    _pos += len;
    return true;
}

void DataStream::write(const enable_serializable &value) {
    value.serialize(*this);
}

bool DataStream::read(enable_serializable &value) {
    return  value.deserialize(*this);
}

bool DataStream::read_memory(char *data, uint32_t len) {
    std::memcpy(data, reinterpret_cast<char*>(&_buf[_pos]), len);
    _pos += len;
    return true;
}

DataStream &DataStream::operator<<(std::string_view value) {
    write(std::string_view(value));
    return *this;
}

DataStream &DataStream::operator>>(std::string &value) {
    read(value);
    return *this;
}