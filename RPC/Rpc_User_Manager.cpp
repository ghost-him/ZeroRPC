//
// Created by ghost-him on 9/11/24.
//

#include "Rpc_User_Manager.h"

void Rpc_User_Manager::create_user(int fd) {
    auto ptr = std::make_shared<Rpc_User>(fd);
    ptr->update_heartbeat();
    std::unique_lock<std::mutex> guard(_lock);
    _user_store[fd] = ptr;
}

void Rpc_User_Manager::update_heartbeat(int fd) {
    std::unique_lock<std::mutex> guard(_lock);
    _user_store[fd]->update_heartbeat();
}

std::vector<int> Rpc_User_Manager::check_invalid_user() {
    std::unique_lock<std::mutex> guard(_lock);
    std::vector<int> ret;
    for (auto& i : _user_store) {
        if (!i.second->is_active()) {
            ret.push_back(i.first);
        }
    }
    return ret;
}

void Rpc_User_Manager::remove_user(int fd) {
    std::unique_lock<std::mutex> guard(_lock);
    if (_user_store.contains(fd)) {
        _user_store.erase(fd);
    }
}
