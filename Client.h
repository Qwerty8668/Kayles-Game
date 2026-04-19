#ifndef KAYLES_CLIENT_H
#define KAYLES_CLIENT_H
#include <chrono>
#include <string>
#include <span>

#include "GameState.h"
#include "Message.h"
#include "WrongMessage.h"

struct sockaddr_in;

class Client {
public:
    Client(std::string &ip_address, uint16_t port, Message message, std::chrono::seconds timeout);

    void run();

    int start_connection();

    struct ::sockaddr_in send_packet(int sockfd, const std::span<const std::byte> &packet);

    static ssize_t wait_for_answer(int sockfd, struct sockaddr_in server_addr, std::span<std::byte> buffer);

    void print_no_answer() const;

    static void print_wrong_answer();

    static void print_game_state(GameState game);

    static void print_wrong_msg(WrongMessage msg);

private:
    std::string server_ip;
    uint16_t server_port;
    Message message;
    const std::chrono::seconds timeout;
};


#endif //KAYLES_CLIENT_H
