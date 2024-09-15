//
// Created by ghost-him on 9/11/24.
//

#pragma once
#include <chrono>

class Rpc_User {
public:
    Rpc_User(int fd);

    int get_ID();

    void update_heartbeat();

    bool is_active();

private:
    int fd;
    std::chrono::high_resolution_clock::time_point last_update_time;

};
