#include <memory>
#include <sys/socket.h>

#include "Server.h"

#include <iostream>

#include "common.h"
#include "Protocol.h"

constexpr size_t BUFFER_SIZE = 1024;

Server::Server(std::array<std::byte, 32> &pawn_row, PawnIndex max_pawn, std::string &ip_address,
               uint16_t port,
               std::chrono::seconds timeout)
    : pawn_row(pawn_row), max_pawn(max_pawn), ip_address(ip_address), port(port), timeout(timeout) {
}

void Server::run() {
    int socket_fd = init_socket();
    std::cout << "Server started." << std::endl;

    std::vector<std::byte> packet(BUFFER_SIZE);
    while (true) {
        sockaddr_in client = receive_packet(socket_fd, packet);
        check_timeouts();

        auto result = Protocol::try_deserialize_request(packet);

        if (std::holds_alternative<uint8_t>(result)) {
            uint8_t error_idx = std::get<uint8_t>(result);
            send_wrong_msg(socket_fd, client, packet, error_idx);
            continue;
        }

        Message msg = std::get<Message>(result);

        auto opt_error_idx = validate_message(msg);
        if (opt_error_idx.has_value()) {
            uint8_t error_idx = opt_error_idx.value();
            send_wrong_msg(socket_fd, client, packet, error_idx);
            continue;
        }

        Game *game_ptr = handle_message(msg);
        if (game_ptr != nullptr)
            send_game_state(socket_fd, client, *game_ptr);
    }
}

int Server::init_socket() const {
    int socket_fd = safe_socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_address = get_address(ip_address.c_str(), port);

    safe_bind(socket_fd, reinterpret_cast<struct sockaddr *>(&server_address),
              sizeof server_address);
    return socket_fd;
}

sockaddr_in Server::receive_packet(int socket_fd, std::vector<std::byte> &buffer) {
    buffer.resize(BUFFER_SIZE);
    struct sockaddr_in client_address{};
    socklen_t socklen = sizeof(client_address);
    size_t received = safe_recvfrom(socket_fd, buffer.data(), buffer.size(), 0,
                                     reinterpret_cast<struct sockaddr *>(&client_address),
                                     &socklen);

    buffer.resize(received);
    return client_address;
}

void Server::check_timeouts() {
    for (auto it = games.begin(); it != games.end();) {
        if (it->second.check_timeout()) {
            free_ids.push(it->first);
            it = games.erase(it);
        } else {
            ++it;
        }
    }
}

Game *Server::handle_message(Message &msg) {
    switch (msg.get_type()) {
        case MessageType::MSG_JOIN:
            return handle_join(msg.get_player_id());
        case MessageType::MSG_MOVE_1:
            return handle_move_1(msg.get_player_id(), msg.get_game_id(), msg.get_pawn());
        case MessageType::MSG_MOVE_2:
            return handle_move_2(msg.get_player_id(), msg.get_game_id(), msg.get_pawn());
        case MessageType::MSG_KEEP_ALIVE:
            return handle_keep_alive(msg.get_player_id(), msg.get_game_id());
        case MessageType::MSG_GIVE_UP:
            return handle_give_up(msg.get_player_id(), msg.get_game_id());
        default:
            return nullptr;
    }
}

std::optional<uint8_t> Server::validate_message(Message &msg) {
    switch (msg.get_type()) {
        case MessageType::MSG_JOIN:
            return validate_join(msg.get_player_id());
        case MessageType::MSG_MOVE_1:
        case MessageType::MSG_MOVE_2:
        case MessageType::MSG_KEEP_ALIVE:
        case MessageType::MSG_GIVE_UP:
            return validate_args(msg.get_player_id(), msg.get_game_id());
        default:
            return static_cast<uint8_t>(0);
    }
}

void Server::send_game_state(int sockfd, struct sockaddr_in client, const Game &game) {
    std::vector<std::byte> buffer = Protocol::serialize_game_state(game);
    safe_sendto(sockfd, buffer.data(), buffer.size(), 0,
                reinterpret_cast<struct sockaddr *>(&client), sizeof(client));
}

void Server::send_wrong_msg(int sockfd, struct sockaddr_in client, std::vector<std::byte> &packet,
                            uint8_t err_idx) {
    std::vector<std::byte> buffer = Protocol::serialize_wrong_msg(packet, err_idx);
    safe_sendto(sockfd, buffer.data(), buffer.size(), 0,
                reinterpret_cast<struct sockaddr *>(&client), sizeof(client));
}

Game *Server::handle_join(PlayerId player_id) {
    Game *game = nullptr;
    if (pending_game_id.has_value()) {
        GameId id = pending_game_id.value();
        game = &games.at(id);
        game->add_player(player_id);
        pending_game_id.reset();
    } else {
        std::optional<GameId> opt_id = generate_id();
        // If no id is free, do nothing.
        if (opt_id.has_value()) {
            GameId id = opt_id.value();
            games.emplace(id, Game(id, player_id, max_pawn, pawn_row, timeout));
            pending_game_id = id;
            game = &games.at(id);
        }
    }
    return game;
}

// We can safely take the game using .at, because message with non-existing game_id
// is taken as invalid. Server is single threaded, so no race-conditions are possible.
Game *Server::handle_move_1(PlayerId player_id, GameId game_id, PawnIndex pawn) {
    Game *game = &games.at(game_id);
    game->make_single_move(pawn, player_id);
    return game;
}

Game *Server::handle_move_2(PlayerId player_id, GameId game_id, PawnIndex pawn) {
    Game *game = &games.at(game_id);
    game->make_double_move(pawn, player_id);
    return game;
}

Game *Server::handle_keep_alive(PlayerId player_id, GameId game_id) {
    Game *game = &games.at(game_id);
    game->keep_alive(player_id);
    return game;
}

Game *Server::handle_give_up(PlayerId player_id, GameId game_id) {
    Game *game = &games.at(game_id);
    game->give_up(player_id);
    return game;
}

std::optional<uint8_t> Server::validate_join(PlayerId player_id) {
    if (player_id == 0) {
        return sizeof(MessageType);
    }
    return std::nullopt;
}

std::optional<uint8_t> Server::validate_args(PlayerId player_id, GameId game_id) {
    size_t pos = sizeof(MessageType);

    if (player_id == 0) return pos;
    pos += sizeof(PlayerId);

    if (!games.contains(game_id)) return pos;

    Game *game = &games.at(game_id);
    if (!game->has_player(player_id)) {
        return pos;
    }

    return std::nullopt;
}

std::optional<GameId> Server::generate_id() {
    if (!free_ids.empty()) {
        GameId id = free_ids.front();
        free_ids.pop();
        return id;
    }

    if (next_game_id < MAX_GAME_ID) {
        return next_game_id++;
    }

    return std::nullopt;
}
