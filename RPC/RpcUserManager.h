//
// Created by ghost-him on 9/11/24.
//

#pragma once

#include "RpcUser.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>

class RpcUserManager {
public:
    void create_user(int fd);

    void remove_user(int fd);

    void update_heartbeat(int fd);

    std::vector<int> check_invalid_user();

private:
    std::unordered_map<int, std::shared_ptr<RpcUser>> _userStore;
    std::mutex lock;
};

