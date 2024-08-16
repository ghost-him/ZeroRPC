//
// Created by ghost-him on 8/11/24.
//

#pragma once

#include "../Core/DataStream.h"
#include "../Core/Compression.h"
#include <string>
#include <utility>

struct RPCRequest : public enable_serializable {
    std::string id;
    std::string method;
    DataStream params;

    SERIALIZE(method, params, id)
};

struct RPCResponse : public enable_serializable {
    DataStream result;
    std::string error;
    std::string id;

    SERIALIZE(result, error, id)
};