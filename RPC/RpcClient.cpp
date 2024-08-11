//
// Created by ghost-him on 8/11/24.
//

#include "RpcClient.h"


RpcClient::RpcClient(std::string_view host, uint16_t port)
: _tcpClient(host, port){
    _thread_pool = &Singleton<ThreadPool>::getInstance();
    _tcpClient.setExecutor([this](std::function<void()> func){
        this->_thread_pool->commit(func, false);
    });

    _tcpClient.setReadMessageCallback([this](SocketChannelPtr channel, DataPtr data){
        json j_response = json::parse(data->data(), data->data() + data->size());
        RPCResponse response = j_response;

        std::lock_guard<std::mutex> lock(_hash_lock);
        auto it = _pending_request.find(response.id);
        if (it != _pending_request.end()) {
            if (!response.error.empty()) {
                it->second.set_exception(std::make_exception_ptr(std::runtime_error(response.error)));
            } else {
                it->second.set_value(response.result);
            }
            _pending_request.erase(it);
        }
    });
}

void RpcClient::run() {
    _tcpClient.run();
}

