//
// Created by ghost-him on 8/12/24.
//
#pragma once

#include <vector>
#include <string_view>
#include <cstdint>
#include <list>
#include <map>
#include <set>
#include <memory>
#include "enable_serializable.h"
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <tuple>

/*
    SupportedDataTypes
    |
    +-- AtomicType
    |   |
    |   +-- PrimitiveType (int, bool, string, etc.)
    |   |
    |   +-- UserDefinedType
    |
    +-- CompositeType (vector, map, list, set, etc.)
 */

template<typename T>
concept UserDefinedType = std::is_base_of_v<enable_serializable, T>;

template<typename T>
concept PrimitiveType  =
        std::is_same_v<T, bool> ||
        std::is_same_v<T, char> ||
        std::is_same_v<T, int32_t> ||
        std::is_same_v<T, int64_t> ||
        std::is_same_v<T, float> ||
        std::is_same_v<T, double> ||
        std::is_same_v<T, std::string> ||
        std::is_same_v<T, const char *> ||
        std::is_same_v<std::remove_const_t<std::remove_extent_t<T>>, char>;

template<typename T>
concept AtomicType = UserDefinedType<T> || PrimitiveType<T>;

template<typename T>
concept CompositeType =
        std::same_as<T, std::vector<typename T::value_type>> ||
        std::same_as<T, std::map<typename T::key_type, typename T::mapped_type>> ||
        std::same_as<T, std::list<typename T::value_type>> ||
        std::same_as<T, std::set<typename T::value_type>> ||
        std::same_as<T, std::unordered_set<typename T::value_type>> ||
        std::same_as<T, std::unordered_map<typename T::key_type, typename T::mapped_type>>;

template<typename T>
concept SupportedDataTypes = AtomicType<T> || CompositeType<T>;

class Data_Stream : public enable_serializable {
public:
    enum Data_Type {
        BOOL = 0,
        CHAR,
        INT32,
        INT64,
        FLOAT,
        DOUBLE,
        STRING,
        VECTOR,
        LIST,
        MAP,
        SET,
        CUSTOM
    };

    Data_Stream() : _pos{0} { is_big_endian = std::endian::native == std::endian::big; }
    ~Data_Stream() = default;

    bool read_memory(char * data, uint32_t len);
    bool read(bool & value);
    bool read(char & value);
    bool read(int32_t & value);
    bool read(int64_t & value);
    bool read(float & value);
    bool read(double & value);
    bool read(std::string & value);
    bool read(enable_serializable& value);

    template <SupportedDataTypes T>
    bool read(std::vector<T>& value);

    template <SupportedDataTypes T>
    bool read(std::list<T>& value);

    template<SupportedDataTypes K, SupportedDataTypes V>
    bool read(std::map<K, V>& value);

    template<SupportedDataTypes T>
    bool read(std::set<T>& value);

    template<SupportedDataTypes K, SupportedDataTypes V>
    bool read(std::unordered_map<K, V>& value);

    template<SupportedDataTypes T>
    bool read(std::unordered_set<T>& value);

    template<SupportedDataTypes T, SupportedDataTypes ...Args>
    bool read_args(T& value, Args&... args);

    void write_memory(const char* data, uint32_t len);
    void write(bool value);
    void write(char value);
    void write(int32_t value);
    void write(int64_t value);
    void write(float value);
    void write(double value);
    void write(const char* value);
    void write(const std::string& data);
    void write(const enable_serializable& value);

    template <SupportedDataTypes T>
    void write(const std::vector<T>& value);

    template <SupportedDataTypes T>
    void write(const std::list<T>& value);

    template<SupportedDataTypes K, SupportedDataTypes V>
    void write(const std::map<K, V>& value);

    template<SupportedDataTypes T>
    void write(const std::set<T>& value);

    template<SupportedDataTypes K, SupportedDataTypes V>
    void write(const std::unordered_map<K, V>& value);

    template<SupportedDataTypes T>
    void write(const std::unordered_set<T>& value);

    template<SupportedDataTypes T, SupportedDataTypes ...Args>
    void write_args(const T& value, const Args&... args);

    template<SupportedDataTypes T>
    Data_Stream & operator << (const T& value);

    Data_Stream & operator<<(const char* value);

    template<SupportedDataTypes T>
    Data_Stream & operator >> (T& value);

    //Data_Stream & operator >> (std::string& value);

    [[nodiscard]] const std::vector<char>& data() const;

    template<SupportedDataTypes... Args>
    std::tuple<Args...> get_args();

    void load(std::string_view data);

    SERIALIZE(is_big_endian, _buf, _pos)

private:
    template<SupportedDataTypes T>
    T get();

    void reserve(int32_t len);
    bool is_big_endian;
    std::vector<char> _buf;
    int32_t _pos;
};

template<SupportedDataTypes... Args>
std::tuple<Args...> Data_Stream::get_args() {
    return std::tuple<Args...>(get<Args>()...);
}

template<SupportedDataTypes T>
T Data_Stream::get() {
    T value;
    read(value);
    return value;
}

template<SupportedDataTypes T>
Data_Stream &Data_Stream::operator<<(const T& value) {
    write(value);
    return *this;
}

template<SupportedDataTypes T>
Data_Stream & Data_Stream::operator >> (T& value) {
    read(value);
    return *this;
}

template<SupportedDataTypes T>
void Data_Stream::write(const std::vector<T> &value) {
    char type = Data_Type::VECTOR;
    write_memory(reinterpret_cast<char*>(&type), sizeof(char));
    int len = value.size();
    write(len);
    for (auto& it : value) {
        write(it);
    }
}

template<SupportedDataTypes T>
void Data_Stream::write(const std::list<T> &value) {
    char type = Data_Type::LIST;
    write_memory(reinterpret_cast<char*>(&type), sizeof(char));
    int len = value.size();
    write(len);
    for (auto& it : value) {
        write(it);
    }
}

template<SupportedDataTypes T>
bool Data_Stream::read(std::list<T> &value) {
    if (_buf[_pos] != Data_Type::LIST) {
        return false;
    }
    value.clear();

    ++_pos;
    int len;
    read(len);
    for (int i = 0; i < len; i ++) {
        T v;
        read(v);
        value.push_back(std::move(v));
    }
    return true;
}

template<SupportedDataTypes T>
bool Data_Stream::read(std::vector<T> &value) {
    if (_buf[_pos] != Data_Type::VECTOR) {
        return false;
    }
    value.clear();

    ++_pos;
    int len;
    read(len);
    for (int i = 0; i < len; i ++) {
        T v;
        read(v);
        value.push_back(std::move(v));
    }
    return true;
}

template<SupportedDataTypes T>
void Data_Stream::write(const std::set<T> &value) {
    char type = Data_Type::SET;
    write_memory(reinterpret_cast<char*>(&type), sizeof(char));
    int len = value.size();
    write(len);
    for (auto& it : value) {
        write(it);
    }
}

template<SupportedDataTypes K, SupportedDataTypes V>
void Data_Stream::write(const std::map<K, V> &value) {
    char type = Data_Type::MAP;
    write_memory(reinterpret_cast<char*>(&type), sizeof(char));
    int len = value.size();
    write(len);
    for (auto& it : value) {
        write(it.first);
        write(it.second);
    }
}

template<SupportedDataTypes T>
bool Data_Stream::read(std::set<T> &value) {
    if (_buf[_pos] != Data_Type::SET) {
        return false;
    }

    ++_pos;
    int len;
    read(len);
    for (int i = 0; i < len; i ++) {
        T v;
        read(v);
        value.insert(v);
    }
    return true;
}

template<SupportedDataTypes K, SupportedDataTypes V>
bool Data_Stream::read(std::map<K, V> &value) {
    if (_buf[_pos] != Data_Type::MAP) {
        return false;
    }

    ++_pos;
    int len;
    read(len);
    for (int i = 0; i < len; i ++) {
        K k;
        read(k);
        V v;
        read(v);
        value[k] = v;
    }
    return true;
}

template<SupportedDataTypes T, SupportedDataTypes... Args>
bool Data_Stream::read_args(T &value, Args &... args) {
    if (!read(value)) {
        return false;
    }
    if constexpr (sizeof...(args) > 0) {
        return read_args(args...);
    }
    return true;
}

template<SupportedDataTypes T, SupportedDataTypes... Args>
void Data_Stream::write_args(const T &value, const Args &... args) {
    write(value);
    if constexpr (sizeof...(args) > 0) {
        write_args(args...);
    }
}

template<SupportedDataTypes T>
bool Data_Stream::read(std::unordered_set<T> &value) {
    if (_buf[_pos] != Data_Type::SET) {
        return false;
    }

    ++_pos;
    int len;
    read(len);
    for (int i = 0; i < len; i ++) {
        T v;
        read(v);
        value.insert(v);
    }
    return true;
}

template<SupportedDataTypes K, SupportedDataTypes V>
bool Data_Stream::read(std::unordered_map<K, V> &value) {
    if (_buf[_pos] != Data_Type::MAP) {
        return false;
    }

    ++_pos;
    int len;
    read(len);
    for (int i = 0; i < len; i ++) {
        K k;
        read(k);
        V v;
        read(v);
        value[k] = v;
    }
    return true;
}

template<SupportedDataTypes T>
void Data_Stream::write(const std::unordered_set<T> &value) {
    char type = Data_Type::SET;
    write_memory(reinterpret_cast<char*>(&type), sizeof(char));
    int len = value.size();
    write(len);
    for (auto& it : value) {
        write(it);
    }
}

template<SupportedDataTypes K, SupportedDataTypes V>
void Data_Stream::write(const std::unordered_map<K, V> &value) {
    char type = Data_Type::MAP;
    write_memory(reinterpret_cast<char*>(&type), sizeof(char));
    int len = value.size();
    write(len);
    for (auto& it : value) {
        write(it.first);
        write(it.second);
    }
}
