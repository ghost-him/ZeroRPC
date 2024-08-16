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
        DataStream value;
        value.load({reinterpret_cast<char*>(data->data()), data->size()});

        RPCRequest request;
        value >> request;

        RPCResponse response;
        response.id = request.id;

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

}

void RpcServer::run() {

    _server.run();
}

void RpcServer::set_compress_algo(CompressionType type) {
    this->_compressionType = type;
}

