#include "Client.h"

#include "Message.h"

Client::Client(std::string &ip_address, uint16_t port, Message message,
               std::chrono::seconds timeout)
    : server_ip(ip_address), server_port(port), message(message), timeout(timeout) {
}

void Client::run() {
    serialize_and_send();
    wait_for_answer();
    print_answer();
}
