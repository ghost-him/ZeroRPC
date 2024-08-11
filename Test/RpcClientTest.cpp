//
// Created by ghost-him on 8/11/24.
//
#include "gtest/gtest.h"
#include "../RPC/RpcClient.h"

TEST(RpcClientTest, initTest) {
    RpcClient client("127.0.0.1", 23333);
    client.run();
    int res = client.call("add", 1, 2);
    std::cerr << "res : " << res << std::endl;
    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

TEST(RpcClientTest, executeTimeConsumingTask) {
    RpcClient client("127.0.0.1", 23333);
    client.run();
    std::cerr << "time consuming task ask:" << std::endl;
    int res = client.call("add", 1, 2);
    std::cerr << "res : " << res << std::endl;
    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}


inline void to_json(json& j, const std::string& r) {
    j = json{{"str", r}};
}

inline void from_json(const json& j, std::string& r) {
    j.at("str").get_to(r);
}

class myClass {
public:
    int a, b;

    myClass operator+(const myClass& other) const {
        myClass ret {
                .a = this->a + other.a,
                .b = this->b + other.b,
        };
        return ret;
    }
};

namespace nlohmann {
    template <>
    struct adl_serializer<myClass> {
        static void to_json(json& j, const myClass& s) {
            j = json{{"a", s.a}, {"b", s.b}};
        }

        static void from_json(const json& j, myClass& s) {
            j.at("a").get_to(s.a);
            j.at("b").get_to(s.b);
        }
    };
}

TEST(RpcClientTest, customClass) {
    RpcClient client("127.0.0.1", 23333);
    client.run();
    myClass a {
        .a = 10,
        .b = 20,
    };
    myClass b {
        .a = 12,
        .b = 30,
    };

    nlohmann::json c = a;

    myClass res = client.call("add3", a, b);
    std::cerr << "res :" << res.a << "  " << res.b << std::endl;

    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
