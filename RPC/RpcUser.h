//
// Created by ghost-him on 9/11/24.
//

#pragma once
#include <chrono>

class RpcUser {
public:
    RpcUser(int fd);

    int getID();

    void updateHeartbeat();

    bool isActive();

private:
    int fd;
    std::chrono::high_resolution_clock::time_point lastUpdateTime;

};
