#include "Server.h"

#include <memory>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "err.h"
#include "common.h"

constexpr size_t BUFFER_SIZE = 1024;

Server::Server(std::array<std::byte, 32> &pawn_row, PawnIndex max_pawn, std::string &ip_address,
               uint16_t port,
               std::chrono::seconds timeout)
    : pawn_row(pawn_row), max_pawn(max_pawn), ip_address(ip_address), port(port), timeout(timeout) {
}

void Server::run() {
    int socket_fd = init_socket();

    while (true) {
        auto [packet, client] = receive_packet(socket_fd);
        check_timeouts();
        std::optional<uint8_t> error_id = validate_packet(packet);
        if (!error_id.has_value()) {
            Message msg = Message::deserialize(packet);
            GameState *game_ptr = handle_message(msg);
            send_game_state(client, game_ptr);
        } else {
            send_wrong_msg(client, packet, error_id.value());
        }
    }
}

int Server::init_socket() const {
    int socket_fd = safe_socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_address = get_server_address(ip_address.c_str(), port);
    safe_bind(socket_fd, reinterpret_cast<struct sockaddr *>(&server_address),
              sizeof server_address);
    return socket_fd;
}

std::pair<std::vector<uint8_t>, sockaddr> Server::receive_packet(int socket_fd) {
    std::vector<uint8_t> buffer(BUFFER_SIZE);

    struct sockaddr client_address;
    socklen_t socklen = sizeof(client_address);

    ssize_t received = recvfrom(socket_fd, buffer.data(), buffer.size(), 0,
                                &client_address, &socklen);
    if (received < 0) syserr("recvfrom");

    buffer.resize(received);
    return {buffer, client_address};
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

std::optional<uint8_t> Server::validate_packet(std::vector<uint8_t> &packet) {
}

GameState *Server::handle_message(Message &msg) {
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
    }
    return nullptr;
}

void Server::send_game_state(struct sockaddr client, GameState *game) {
}

void Server::send_wrong_msg(struct sockaddr client, std::vector<uint8_t> &packet, uint8_t err_idx) {
}

GameState *Server::handle_join(PlayerId player_id) {
    GameState *game = nullptr;
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
            games.emplace(id, GameState(id, player_id, max_pawn, pawn_row, timeout));
            pending_game_id = id;
            game = &games.at(id);
        }
    }
    return game;
}

// We can safely take the game using .at, because message with non-existing game_id
// is taken as invalid. Server is single threaded, so no race-conditions are possible.
GameState *Server::handle_move_1(PlayerId player_id, GameId game_id, PawnIndex pawn) {
    GameState *game = &games.at(game_id);
    game->make_single_move(pawn, player_id);
    return game;
}

GameState *Server::handle_move_2(PlayerId player_id, GameId game_id, PawnIndex pawn) {
    GameState *game = &games.at(game_id);
    game->make_double_move(pawn, player_id);
    return game;
}

GameState *Server::handle_keep_alive(PlayerId player_id, GameId game_id) {
    GameState *game = &games.at(game_id);
    game->keep_alive(player_id);
    return game;
}

GameState *Server::handle_give_up(PlayerId player_id, GameId game_id) {
    GameState *game = &games.at(game_id);
    game->give_up(player_id);
    return game;
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
