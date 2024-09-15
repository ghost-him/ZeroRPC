//
// Created by ghost-him on 8/10/24.
//

#include <string>
#include <string_view>
#include "Network.h"
#include <cstdint>
#include "Channel.h"
#include <atomic>

class Tcp_Client : public Network {
public:
    Tcp_Client(std::string_view addr, uint16_t port);

    void run() override;

    void send_message(std::string_view data);

    void stop();

    ~Tcp_Client() override = default;

private:
    void read_data();

    void disconnect();
    int _fd;
    std::string _ip;
    uint16_t _port;
    SocketChannelPtr _channel;
    std::atomic<bool> _running;
};