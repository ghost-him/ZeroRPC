//
// Created by ghost-him on 8/11/24.
//
#include "gtest/gtest.h"
#include "../RPC/RpcServer.h"
#include "../RPC/FunctionHandler.hpp"


int add(int a, int b) {
    return a + b;
}

std::string append(std::string a, std::string b) {
    return a + b;
}

TEST (RpcServerTest, initTest) {
    RpcServer server(23333);
    server.registerMethod("add", add);
    server.registerMethod("append", append);
    server.run();
}

int add2(int a, int b) {
    std::cerr << "time consuming" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds (3));
    return a + b;
}

TEST (RpcServerTest, executeTimeConsumingTask) {
    RpcServer server(23333);
    server.registerMethod("add", add2);
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
    RpcServer server(23333);
    server.registerMethod("add4", add4);
    server.run();
}

TEST (RpcServerText, testCompression) {
    RpcServer server(23333);
    server.set_compress_algo(CompressionType::Brotli);
    server.registerMethod("add", add);
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

}