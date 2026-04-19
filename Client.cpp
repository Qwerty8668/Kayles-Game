#include "Client.h"

#include <bitset>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iomanip>

#include "common.h"
#include "GameState.h"
#include "Message.h"
#include "Protocol.h"

Client::Client(std::string &ip_address, uint16_t port, Message message,
               std::chrono::seconds timeout)
    : server_ip(ip_address), server_port(port), message(message), timeout(timeout) {
}

void Client::run() {
    int socket_fd = start_connection();

    std::vector<std::byte> packet = Protocol::serialize_request(message);

    struct sockaddr_in server_addr = send_packet(socket_fd, packet);

    std::optional<std::vector<std::byte>> answer = wait_for_answer(socket_fd, server_addr);

    if (!answer.has_value()) {
        print_no_answer();
        return;
    }

    auto response = Protocol::try_deserialize_response(answer.value());

    if (!response.has_value()) {
        print_wrong_answer();
    }

    auto response_val = response.value();

    if (std::holds_alternative<GameState>(response_val)) {
        GameState state = std::get<GameState>(response_val);
        print_game_state(state);
    } else {
        WrongMessage wrng_msg = std::get<WrongMessage>(response_val);
        print_wrong_msg(wrng_msg);
    }
}

int Client::start_connection() {
    int socket_fd = safe_socket(AF_INET, SOCK_DGRAM, 0);
    struct timeval tv;
    tv.tv_sec = timeout.count();
    tv.tv_usec = 0;

    safe_setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    return socket_fd;
}

struct sockaddr_in Client::send_packet(int sockfd, const std::vector<std::byte> &packet) {
    struct sockaddr_in server = get_address(server_ip.c_str(), server_port);
    safe_sendto(sockfd, packet.data(), packet.size(), 0,
                reinterpret_cast<struct sockaddr *>(&server), sizeof(server));
    return server;
}

std::optional<std::vector<std::byte>> Client::wait_for_answer(int sockfd, struct sockaddr_in server_addr) {
    std::vector<std::byte> buffer(BUFFER_SIZE);
    safe_connect(sockfd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr));
    ssize_t received = safe_timeout_recv(sockfd, buffer.data(), buffer.size(), 0);
    if (received == -1) {
        return std::nullopt;
    }
    buffer.resize(received);
    return buffer;

}

void Client::print_no_answer() const {
    std::cout <<"No answer received from the server in " << timeout.count() << " second(s)\n";
}

void Client::print_wrong_answer() {
    std::cout << "Couldn't parse response from the server.\n";
}

void Client::print_game_state(GameState game) {
    std::cout << "=== GameState [ID: " << game.game_id << "] ===\n"
       << "  Status      : " << static_cast<int>(game.status) << "\n"
       << "  Player A ID : " << game.player_a_id << "\n"
       << "  Player B ID : " << game.player_b_id << "\n"
       << "  Max Pawn    : " << static_cast<unsigned>(game.max_pawn) << "\n"
       << "  Pawn Row    : ";

    size_t bytes_to_print = (game.max_pawn / 8) + 1;

    for (size_t i = 0; i < bytes_to_print; ++i) {
        std::cout << std::bitset<8>(static_cast<unsigned long long>(game.pawn_row[i])) << " ";
    }

    std::cout << "\n=============================";
}

void Client::print_wrong_msg(WrongMessage msg) {
    std::cout << "=== WrongMessage [Error on index: "
              << static_cast<unsigned>(msg.error_idx) << "] ===\n"
              << "  Status      : " << static_cast<unsigned>(msg.status) << "\n"
              << "  First " << msg.first_bytes.size() << " bytes:\n  ";

    std::cout << std::hex << std::uppercase << std::setfill('0');

    for (size_t i = 0; i < msg.first_bytes.size(); ++i) {
        std::cout << "0x" << std::setw(2) << static_cast<unsigned>(msg.first_bytes[i]) << " ";

        if (i == 5) std::cout << "\n  ";
    }

    std::cout << std::dec << "\n=========================================\n";
}