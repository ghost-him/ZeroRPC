//
// Created by ghost-him on 9/11/24.
//

#include "RpcUserManager.h"

void RpcUserManager::create_user(int fd) {
    auto ptr = std::make_shared<RpcUser>(fd);
    ptr->updateHeartbeat();
    std::unique_lock<std::mutex> guard(lock);
    _userStore[fd] = ptr;
}

void RpcUserManager::update_heartbeat(int fd) {
    std::unique_lock<std::mutex> guard(lock);
    _userStore[fd]->updateHeartbeat();
}

std::vector<int> RpcUserManager::check_invalid_user() {
    std::unique_lock<std::mutex> guard(lock);
    std::vector<int> ret;
    for (auto& i : _userStore) {
        if (!i.second->isActive()) {
            ret.push_back(i.first);
        }
    }
    return ret;
}

void RpcUserManager::remove_user(int fd) {
    std::unique_lock<std::mutex> guard(lock);
    if (_userStore.contains(fd)) {
        _userStore.erase(fd);
    }
}
