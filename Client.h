#ifndef KAYLES_CLIENT_H
#define KAYLES_CLIENT_H
#include <chrono>
#include <cstdint>
#include <string>

#include "Message.h"

class Client {
public:
    Client(std::string &ip_address, uint16_t port, Message message, std::chrono::seconds timeout);

    void run();

private:
    std::string server_ip;
    uint16_t server_port;
    Message message;
    const std::chrono::seconds timeout;
};


#endif //KAYLES_CLIENT_H
