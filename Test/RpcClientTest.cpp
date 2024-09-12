//
// Created by ghost-him on 8/11/24.
//
#include "gtest/gtest.h"
#include "../RPC/RpcClient.h"

TEST(RpcClientTest, testRPCStruct) {
    DataStream ds;
    RPCResponse response;
    DataStream ans;
    ans << 1;
    response.error = "123";
    response.result = ans;
    response.id = "1.23";
    ds << response;

    DataStream recv;
    recv.load({ds.data().data(), ds.data().size()});

    RPCResponse recv_response;
    recv >> recv_response;
    ASSERT_EQ(recv_response.error, response.error);

}

TEST(RpcClientTest, initTest) {
    RpcClient client("127.0.0.1", 23333);
    client.run();
    int res = client.call<int>("add", 1, 2);
    std::cerr << "res : " << res << std::endl;
    std::string appendedString = client.call<std::string>("append", "hello ", "world");
    std::cerr << "ans: " << appendedString << std::endl;

    std::string order = client.call<std::string>("pargOrder", 123, "hello");
    std::cerr << "order: " << order << std::endl;

    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

TEST(RpcClientTest, executeTimeConsumingTask) {
    RpcClient client("127.0.0.1", 23333);
    client.run();
    std::cerr << "time consuming task ask:" << std::endl;
    int res = client.call<int>("add", 1, 2);
    std::cerr << "res : " << res << std::endl;
    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
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

TEST(RpcClientTest, testCustomClass) {
    RpcClient client("127.0.0.1", 23333);
    client.run();
    myClass a, b;
    a.a = 10;
    a.b = 20;
    b.a = 30;
    b.b = 40;
    auto ret = client.call<myClass>("add4", a, b);
    std::cout << ret.a << " " << ret.b << std::endl;
    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

TEST(RpcClientTest, testCompression) {
    RpcClient client("127.0.0.1", 23333);
    client.set_compress_algo(CompressionType::Brotli);
    client.run();
    int res = client.call<int>("add", 1, 2);
    std::cerr << "res : " << res << std::endl;
    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

TEST(RpcClientTest, testMemberFunc) {
    RpcClient client("127.0.0.1", 23333);
    client.run();
    while(1) {
        std::cout << client.call<int32_t>("requestID") << std::endl;
        std::cout << "send test message" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds (1000));
    }
}

TEST(RpcClientTest, testHeartbeat) {
    // 该测试需要在源码中删除部分代码来测试：
    // RPC/RpcClient.cpp 第39-41行代码，该代码为定时器，用于向服务器发出心跳。
    // 服务器运行 TEST (RpcServerTest, initTest)
    RpcClient client("127.0.0.1", 23333);
    client.run();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    int res = client.call<int>("add", 1, 2);
    std::cerr << "res : " << res << std::endl;
    // 发送成功

    std::this_thread::sleep_for(std::chrono::seconds (30));
    std::clog << "向服务器发送消息" << std::endl;
    res = client.call<int>("add", 2, 3);
    std::cerr << "res : " << res << std::endl;
    // 如果发送失败，则测试成功：在等待30s以后，服务器已关闭连接，此时应该无法向服务器发送消息
}