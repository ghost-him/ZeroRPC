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

TEST(RpcClientText, testCompression) {
    RpcClient client("127.0.0.1", 23333);
    client.set_compress_algo(CompressionType::Brotli);
    client.run();
    int res = client.call<int>("add", 1, 2);
    std::cerr << "res : " << res << std::endl;
    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}