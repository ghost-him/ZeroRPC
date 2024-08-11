//
// Created by ghost-him on 8/11/24.
//
#include "gtest/gtest.h"
#include "../RPC/RpcServer.h"

int add(int a, int b) {
    return a + b;
}

TEST (RpcServerTest, initTest) {
    RpcServer server(23333);
    server.resigterMethod("add", add);
    server.run();
}

int add2(int a, int b) {
    std::cerr << "time consuming" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds (3));
    return a + b;
}

TEST (RpcServerTest, executeTimeConsumingTask) {
    RpcServer server(23333);
    server.resigterMethod("add", add2);
    server.run();
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
myClass add3(myClass a, myClass b) {
    return a + b;
}

TEST (RpcServerTest, customClass) {
    RpcServer server(23333);
    server.resigterMethod("add3", add3);
    server.run();
}

/*
 * 无法调用无返回值的函数
 *
void print(int a) {
    std::cerr << a << std::endl;
}

TEST (RpcServerTest, noRetFunc) {
    RpcServer server(23333);
    server.resigterMethod("noret", print);
}

 */