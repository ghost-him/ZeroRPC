//
// Created by ghost-him on 8/11/24.
//

#include "RpcServer.h"
#include <stdexcept>

RpcServer::RpcServer(int port)
: _server{port, static_cast<int>(std::thread::hardware_concurrency() / 2)} {
    _threadPool = &Singleton<ThreadPool>::getInstance();

    _server.setExecutor([this](std::function<void()> func){
        this->_threadPool->commit(func, false);
    });

    _server.setNewConnectionCallback([this](int fd){
        _rpcUserManager.create_user(fd);
        std::clog << "添加一个用户：" << fd << std::endl;
    });

    _server.setDisconnectCallback([this](int fd){
        _rpcUserManager.remove_user(fd);
        std::clog << "删除一个用户：" << fd << std::endl;
    });

    _server.setReadMessageCallback([this](SocketChannelPtr channel, DataPtr data){
        // std::string request_str {reinterpret_cast<const char*>(data->data()), data->size()};
        DataStream value;
        value.load({reinterpret_cast<char*>(data->data()), data->size()});

        RPCRequest request;
        value >> request;

        RPCResponse response;
        response.id = request.id;

        _rpcUserManager.update_heartbeat(channel->getFd());
        try {
            auto result = _manager.call(request.method, request.params);

            if (result.has_value()) {
                response.result = result.value();
            } else {
                response.error = request.method + " method not found";
            }
        } catch (const std::exception& e) {
            response.error = e.what();
        }

        DataStream response_buf;
        response_buf << response;
        const auto& sendMessage = response_buf.data();
        channel->writeData({reinterpret_cast<const std::byte*>(sendMessage.data()), sendMessage.size()});
    });

    this->registerMethod(HEARTBEAT_SIG, [this](){
        this->handle_heartbeat_signal();
    });

    _timer.setExecutor([this](std::function<void()> func){
        this->_threadPool->commit(func, false);
    });

    _timer.setPeriodicTimer([this](){
        std::clog << "检测一次用户信息" << std::endl;
        auto ret = _rpcUserManager.check_invalid_user();
        // 通知tcpserver关闭连接
        for (auto& i : ret) {
            _server.closeConnection(i);
        }
        }, std::chrono::seconds(HEARTBEAT_TIMEOUT));

}

void RpcServer::run() {
    _threadPool->commit([this](){
        _timer.run();
        }, false);
    _server.run();
}

void RpcServer::set_compress_algo(CompressionType type) {
    this->_compressionType = type;
}

void RpcServer::handle_heartbeat_signal() {
    std::clog << "收到一个心跳信息" << std::endl;
}

