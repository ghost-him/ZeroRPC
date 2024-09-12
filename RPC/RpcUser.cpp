//
// Created by ghost-him on 9/11/24.
//

#include "RpcUser.h"
#include "../Common/prg_cfg.hpp"

int RpcUser::getID() {
    return this->fd;
}

void RpcUser::updateHeartbeat() {
    this->lastUpdateTime = std::chrono::high_resolution_clock::now();
}

bool RpcUser::isActive() {
    return std::chrono::high_resolution_clock::now() - this->lastUpdateTime <= std::chrono::seconds(HEARTBEAT_TIMEOUT);
}

RpcUser::RpcUser(int fd) {
    this->fd = fd;
}
