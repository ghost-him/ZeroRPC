//
// Created by ghost-him on 9/11/24.
//

#include "Rpc_User.h"
#include "../Common/prg_cfg.hpp"

int Rpc_User::get_ID() {
    return this->fd;
}

void Rpc_User::update_heartbeat() {
    this->last_update_time = std::chrono::high_resolution_clock::now();
}

bool Rpc_User::is_active() {
    return std::chrono::high_resolution_clock::now() - this->last_update_time <= std::chrono::seconds(HEARTBEAT_TIMEOUT);
}

Rpc_User::Rpc_User(int fd) {
    this->fd = fd;
}
