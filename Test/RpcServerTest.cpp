//
// Created by ghost-him on 8/11/24.
//
#include "gtest/gtest.h"
#include "../RPC/Rpc_Server.h"
#include "../RPC/Function_Handler.hpp"


int add(int a, int b) {
    return a + b;
}

std::string append(std::string a, std::string b) {
    return a + b;
}

std::string testPargsOrder(int a, std::string b) {
    // 用于测试参数的顺序
    return std::to_string(a) + b;
}

TEST (RpcServerTest, initTest) {
    Rpc_Server server(23333);
    server.register_method("add", add);
    server.register_method("append", append);
    server.register_method("pargOrder", testPargsOrder);
    server.run();
}

int add2(int a, int b) {
    std::cerr << "time consuming" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds (3));
    return a + b;
}

TEST (RpcServerTest, executeTimeConsumingTask) {
    Rpc_Server server(23333);
    server.register_method("add", add2);
    server.run();
}

class myClass :public enable_serializable {
public:
    int a, b;
    myClass operator+(const myClass& other) const {
        myClass ret;
        ret.a = this->a + other.a;
        ret.b = this->b + other.b;
        return ret;
    }
    SERIALIZE(a, b)
};

myClass add4(myClass a, myClass b) {
    return a + b;
}

TEST (RpcServerTest, customClass) {
    Rpc_Server server(23333);
    server.register_method("add4", add4);
    server.run();
}

TEST (RpcServerText, testCompression) {
    Rpc_Server server(23333);
    server.set_compress_algo(Compression_Type::Brotli);
    server.register_method("add", add);
    server.run();
}

class Generator {
public:
    int get_new_id() {
        return _id++;
    }
private:
    int _id { 1 };
};

TEST (RpcServerTest, testMemberFunc) {
    Rpc_Server server(23333);
    Generator gen;
    server.register_method("requestID", [&]() {
        return gen.get_new_id();
    });
    server.run();
}