//
// Created by ghost-him on 8/9/24.
//

#include "Channel.h"
#include "TcpServer.h"
#include <cstring>

SocketChannel::SocketChannel(int fd, Network* server)
:fd{fd}, server{server}, readBufferLength{0} {

}

bool SocketChannel::writeData(std::span<const std::byte> data) {
    DataPtr ptr = std::make_shared<std::vector<std::byte>>(data.size() + HEAD_LENGTH);

    auto memHead = variant2mem<int32_t>(data.size());
    memcpy(ptr->data(), memHead.data(), HEAD_LENGTH);
    memcpy(ptr->data() + HEAD_LENGTH, data.data(), data.size());

    sendMessages.push(ptr);
    server->_onSendMessage(shared_from_this());

    return true;
}

void SocketChannel::parseMessage() {
    while(1) {
        if (readBufferLength == 0) {
            if (readBuffer.size() > HEAD_LENGTH) {
                //表明当前已经有一个用于存放长度的值
                std::array<std::byte, HEAD_LENGTH> temp;
                readBuffer.dequeue(temp.begin(), HEAD_LENGTH);
                readBufferLength = mem2variant<int32_t>(temp, 1, BUFFER_SIZE / 2);
            } else {
                break;
            }
        }

        if (readBuffer.size() >= readBufferLength) {
            DataPtr newData = std::make_shared<std::vector<std::byte>>(readBufferLength);
            readBuffer.dequeue(newData->data(), readBufferLength);
            readBufferLength = 0;
            readMessages.push(std::move(newData));
        }
    }
    while (!readMessages.empty()) {
        auto DataPtr = readMessages.front_pop();
        if (server->_onReadMessage) {
            server->_onReadMessage(shared_from_this(), DataPtr);
        }
    }
}
