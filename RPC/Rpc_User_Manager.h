//
// Created by ghost-him on 9/11/24.
//

#pragma once

#include "Rpc_User.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>

class Rpc_User_Manager {
public:
    void create_user(int fd);

    void remove_user(int fd);

    void update_heartbeat(int fd);

    std::vector<int> check_invalid_user();

private:
    std::unordered_map<int, std::shared_ptr<Rpc_User>> _user_store;
    std::mutex _lock;
};

