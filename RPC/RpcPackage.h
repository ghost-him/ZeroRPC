//
// Created by ghost-him on 8/11/24.
//

#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <utility>

using json = nlohmann::json;
using RpcMethod = std::function<json(const std::vector<json>&)>;

struct RPCRequest {
    std::string method;
    std::vector<json> params;
    std::string id;
};

struct RPCResponse {
    json result;
    std::string error;
    std::string id;
};

struct String {
    std::string value;
    String(std::string  s) : value(std::move(s)) {}
    operator std::string() const { return value; }
};

namespace nlohmann {
    template <>
    struct adl_serializer<RPCRequest> {
        static void to_json(json& j, const RPCRequest& r) {
            j = json{{"method", r.method}, {"params", r.params}, {"id", r.id}};
        }

        static void from_json(const json& j, RPCRequest& r) {
            j.at("method").get_to(r.method);
            j.at("params").get_to(r.params);
            j.at("id").get_to(r.id);
        }
    };

    template <>
    struct adl_serializer<RPCResponse> {
        static void to_json(json& j, const RPCResponse& r) {
            j = json{{"result", r.result}, {"error", r.error}, {"id", r.id}};
        }

        static void from_json(const json& j, RPCResponse& r) {
            j.at("result").get_to(r.result);
            j.at("error").get_to(r.error);
            j.at("id").get_to(r.id);
        }
    };

    template <>
    struct adl_serializer<String> {
        static void to_json(json& j, const String& s) {
            j = json{{"value", s}};
        }

        static void from_json(const json& j, String& s) {
            j.at("value").get_to(s);
        }
    };
}
