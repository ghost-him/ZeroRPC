//
// Created by ghost-him on 8/11/24.
//

#include "Rpc_Server.h"
#include <stdexcept>

Rpc_Server::Rpc_Server(int port)
: _server{port, static_cast<int>(std::thread::hardware_concurrency() / 2)} {
    _threadPool = &Singleton<Thread_Pool>::getInstance();

    _server.set_executor([this](std::function<void()> func){
        this->_threadPool->commit(func, false);
    });

    _server.set_new_connection_callback([this](int fd){
        _rpcUserManager.create_user(fd);
        std::clog << "添加一个用户：" << fd << std::endl;
    });

    _server.set_disconnect_callback([this](int fd){
        _rpcUserManager.remove_user(fd);
        std::clog << "删除一个用户：" << fd << std::endl;
    });

    _server.set_read_message_callback([this](SocketChannelPtr channel, DataPtr data){
        // std::string request_str {reinterpret_cast<const char*>(data->data()), data->size()};
        Data_Stream value;
        value.load({reinterpret_cast<char*>(data->data()), data->size()});

        RPCRequest request;
        value >> request;

        RPCResponse response;
        response.id = request.id;

        _rpcUserManager.update_heartbeat(channel->get_fd());
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

        Data_Stream response_buf;
        response_buf << response;
        const auto& sendMessage = response_buf.data();
        channel->write_data({reinterpret_cast<const std::byte*>(sendMessage.data()), sendMessage.size()});
    });

    this->register_method(HEARTBEAT_SIG, [this]() {
        this->handle_heartbeat_signal();
    });

    _timer.set_executor([this](std::function<void()> func){
        this->_threadPool->commit(func, false);
    });

    _timer.set_periodic_timer([this](){
        std::clog << "检测一次用户信息" << std::endl;
        auto ret = _rpcUserManager.check_invalid_user();
        // 通知tcpserver关闭连接
        for (auto& i : ret) {
            _server.close_connection(i);
        }
        }, std::chrono::seconds(HEARTBEAT_TIMEOUT));

}

void Rpc_Server::run() {
    _threadPool->commit([this](){
        _timer.run();
        }, false);
    _server.run();
}

void Rpc_Server::set_compress_algo(Compression_Type type) {
    this->_compressionType = type;
}

Thread_Pool& Rpc_Server::get_thread_pool() {
    return *this->_threadPool;
}

Timer& Rpc_Server::get_timer() {
    return this->_timer;
}

void Rpc_Server::handle_heartbeat_signal() {
    std::clog << "收到一个心跳信息" << std::endl;
}

