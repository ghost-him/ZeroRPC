//
// Created by ghost-him on 8/12/24.
//

#pragma once

class DataStream;

class enable_serializable {
public:
    virtual void serialize(DataStream &stream) const = 0;

    virtual bool deserialize(DataStream &stream) = 0;
};

#define SERIALIZE(...) \
    void serialize(DataStream & stream) const \
    {                  \
        char type = DataStream::CUSTOM;       \
        stream.write_memory(reinterpret_cast<char*>(&type), sizeof(char)); \
        stream.write_args(__VA_ARGS__);\
    }\
\
    bool deserialize(DataStream & stream)     \
    {                  \
        char type;\
        stream.read_memory(reinterpret_cast<char*>(&type), sizeof(char));     \
        if (type != DataStream::CUSTOM) {     \
            return false;               \
        }              \
        stream.read_args(__VA_ARGS__);        \
        return true;\
    }
