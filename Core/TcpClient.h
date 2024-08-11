//
// Created by ghost-him on 8/10/24.
//

#include <string>
#include <string_view>
#include "Network.h"
#include <cstdint>
#include "Channel.h"
#include <atomic>

class TcpClient : public Network {
public:
    TcpClient(std::string_view addr, uint16_t port);

    void run() override;

    void sendMessage(std::string_view data);

    ~TcpClient() override = default;

private:
    void readData();

    void disconnect();
    int _fd;
    std::string _ip;
    uint16_t _port;
    SocketChannelPtr _channel;
    std::atomic<bool> _running;
};