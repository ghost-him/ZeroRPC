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

    _server.setReadMessageCallback([this](SocketChannelPtr channel, DataPtr data){
        // std::string request_str {reinterpret_cast<const char*>(data->data()), data->size()};
        json json_request = json::parse(data->data(), data->data() + data->size());
        RPCRequest request = json_request;

        RPCResponse response;
        response.id = request.id;

        if (_methods.contains(request.method)) {
            // 存在这个方法
            try {
                response.result = _methods[request.method](request.params);
            } catch (const std::exception& e) {
                response.error = e.what();
            }
        } else {
            // 不存在这个方法
            response.error = request.method + " method not found";
        }

        json json_response = response;
        const auto& sendMessage = json_response.dump();
        channel->writeData({reinterpret_cast<const std::byte*>(sendMessage.data()), sendMessage.size()});
    });

}

void RpcServer::run() {

    _server.run();
}

