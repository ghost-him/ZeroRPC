//
// Created by ghost-him on 8/12/24.
//

#pragma once

class Data_Stream;

class enable_serializable {
public:
    virtual void serialize(Data_Stream &stream) const = 0;

    virtual bool deserialize(Data_Stream &stream) = 0;
};

#define SERIALIZE(...) \
    void serialize(Data_Stream & stream) const \
    {                  \
        char type = Data_Stream::CUSTOM;       \
        stream.write_memory(reinterpret_cast<char*>(&type), sizeof(char)); \
        stream.write_args(__VA_ARGS__);\
    }\
\
    bool deserialize(Data_Stream & stream)     \
    {                  \
        char type;\
        stream.read_memory(reinterpret_cast<char*>(&type), sizeof(char));     \
        if (type != Data_Stream::CUSTOM) {     \
            return false;               \
        }              \
        stream.read_args(__VA_ARGS__);        \
        return true;\
    }
