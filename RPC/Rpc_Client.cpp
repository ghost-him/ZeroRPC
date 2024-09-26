//
// Created by ghost-him on 8/11/24.
//

#include "Rpc_Client.h"


Rpc_Client::Rpc_Client(std::string_view host, uint16_t port)
: _tcpClient(host, port){
    _thread_pool = &Singleton<Thread_Pool>::getInstance();
    _tcpClient.set_executor([this](std::function<void()> func) {
        this->_thread_pool->commit(func, false);
    });

    _tcpClient.set_read_message_callback([this](SocketChannelPtr channel, DataPtr data) {
        Data_Stream response_buf;
        response_buf.load({(char *) data->data(), data->size()});

        RPCResponse response;
        response_buf >> response;

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

    _timer.set_executor([this](std::function<void()> func) {
        this->_thread_pool->commit(func, false);
    });

    _timer.set_periodic_timer([this]() {
        this->heartbeat_signal();
    }, std::chrono::seconds(HEARTBEAT_REPEAT_TIME));

}

void Rpc_Client::run() {
    _tcpClient.run();
    this->_thread_pool->commit([this](){
        _timer.run();
    }, false);
}

void Rpc_Client::set_compress_algo(Compression_Type type) {
    this->_compressionType = type;
}

Thread_Pool & Rpc_Client::get_thread_pool() {
    return *this->_thread_pool;
}

Timer & Rpc_Client::get_timer() {
    return this->_timer;
}

void Rpc_Client::heartbeat_signal() {
    this->call<void>(HEARTBEAT_SIG);
}

Rpc_Client::~Rpc_Client() {
    // todo
    _tcpClient.stop();
    _timer.stop();
}
