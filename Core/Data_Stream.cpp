//
// Created by ghost-him on 8/12/24.
//

#include "Data_Stream.h"
#include <cstring>
#include <iostream>
#include <bit>
#include <algorithm>

void Data_Stream::write_memory(const char* data, uint32_t len) {
    reserve(len);
    auto size {_buf.size()};
    _buf.resize(size + len);

    std::memcpy(&_buf[size], data, len);
}

void Data_Stream::reserve(int32_t len) {
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

void Data_Stream::write(bool value) {
    char type {Data_Type::BOOL};
    write_memory(&type, sizeof(char));
    write_memory(reinterpret_cast<char *>(&value), sizeof(bool));
}

void Data_Stream::write(char value) {
    char type {Data_Type::CHAR};
    write_memory(&type, sizeof(char));
    write_memory(reinterpret_cast<char *>(&value), sizeof(char));
}

void Data_Stream::write(int32_t value) {
    char type {Data_Type::INT32};
    write_memory(&type, sizeof(char));
    if (is_big_endian) {
        char * first = reinterpret_cast<char*>(&value);
        char * last = first + sizeof(int32_t);
        std::reverse(first, last);
    }
    write_memory(reinterpret_cast<char *>(&value), sizeof(int32_t));
}

void Data_Stream::write(int64_t value) {
    char type {Data_Type::INT64};
    write_memory(&type, sizeof(char));
    if (is_big_endian) {
        char * first = reinterpret_cast<char*>(&value);
        char * last = first + sizeof(int64_t);
        std::reverse(first, last);
    }
    write_memory(reinterpret_cast<char *>(&value), sizeof(int64_t));
}

void Data_Stream::write(float value) {
    char type {Data_Type::FLOAT};
    write_memory(&type, sizeof(char));
    if (is_big_endian) {
        char * first = reinterpret_cast<char*>(&value);
        char * last = first + sizeof(float);
        std::reverse(first, last);
    }
    write_memory(reinterpret_cast<char *>(&value), sizeof(float));
}

void Data_Stream::write(double value) {
    char type {Data_Type::DOUBLE};
    write_memory(&type, sizeof(char));
    if (is_big_endian) {
        char * first = reinterpret_cast<char*>(&value);
        char * last = first + sizeof(double);
        std::reverse(first, last);
    }
    write_memory(reinterpret_cast<char *>(&value), sizeof(double));
}

void Data_Stream::write(const std::string&  data) {
    char type {Data_Type::STRING};
    write_memory(&type, sizeof(char));
    write(static_cast<int32_t>(data.size()));
    write_memory(data.data(), data.size());
}

bool Data_Stream::read(bool &value) {
    if (_buf[_pos] != Data_Type::BOOL) {
        return false;
    }
    ++_pos;
    value = _buf[_pos];
    ++_pos;
    return true;
}

bool Data_Stream::read(char &value) {
    if (_buf[_pos] != Data_Type::CHAR) {
        return false;
    }
    ++_pos;
    value = _buf[_pos];
    ++_pos;
    return true;
}

bool Data_Stream::read(int32_t &value) {
    if (_buf[_pos] != Data_Type::INT32) {
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

bool Data_Stream::read(int64_t &value) {
    if (_buf[_pos] != Data_Type::INT64) {
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

bool Data_Stream::read(float &value) {
    if (_buf[_pos] != Data_Type::FLOAT) {
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

bool Data_Stream::read(double &value) {
    if (_buf[_pos] != Data_Type::DOUBLE) {
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

bool Data_Stream::read(std::string &value) {
    if (_buf[_pos] != Data_Type::STRING) {
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

void Data_Stream::write(const enable_serializable &value) {
    value.serialize(*this);
}

bool Data_Stream::read(enable_serializable &value) {
    return  value.deserialize(*this);
}

bool Data_Stream::read_memory(char *data, uint32_t len) {
    std::memcpy(data, reinterpret_cast<char*>(&_buf[_pos]), len);
    _pos += len;
    return true;
}

Data_Stream &Data_Stream::operator<<(const char* value) {
    write(value);
    return *this;
}
/*
Data_Stream &Data_Stream::operator>>(std::string &value) {
    read(value);
    return *this;
}
*/
const std::vector<char> &Data_Stream::data() const {
    return _buf;
}

void Data_Stream::write(const char *value) {
    write({value, strlen(value)});
}

void Data_Stream::load(std::string_view data) {
    reserve(data.size());
    std::memcpy(_buf.data(), data.data(), data.size());
    _pos = 0;
}
