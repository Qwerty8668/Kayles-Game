#include "Server.h"

#include <memory>

Server::Server(std::array<std::byte, 32>& pawn_row, PawnIndex max_pawn, std::string& ip_address, uint16_t port, std::chrono::seconds timeout)
    : pawn_row(pawn_row), max_pawn(max_pawn), ip_address(ip_address), port(port), timeout(timeout) {

}

void Server::run() {
    init_socket();

    while (true) {
        Message msg = receive_and_deserialize();
        check_timeouts();
        handle_message(msg);
    }
}

void Server::init_socket() {

}

Message Server::receive_and_deserialize() {

}

void Server::handle_message(Message msg) {

    switch (msg.get_type()) {
        case MessageType::MSG_JOIN:
            handle_join(msg.get_player_id());
            break;
        case MessageType::MSG_MOVE_1:
            handle_move_1(msg.get_player_id(), msg.get_game_id(), msg.get_pawn());
            break;
        case MessageType::MSG_MOVE_2:
            handle_move_2(msg.get_player_id(), msg.get_game_id(), msg.get_pawn());
            break;
        case MessageType::MSG_KEEP_ALIVE:
            handle_keep_alive(msg.get_player_id(), msg.get_game_id());
            break;
        case MessageType::MSG_GIVE_UP:
            handle_give_up(msg.get_player_id(), msg.get_game_id());
            break;
    }
}

void Server::handle_join(PlayerId player_id) {
    if (pending_game_id.has_value()) {
        GameId id = pending_game_id.value();
        GameState& game = games.at(id);
        game.add_player(player_id);
        pending_game_id.reset();
    } else {
        std::optional<GameId> opt_id = generate_id();
        // If no id is free, do nothing.
        if (opt_id.has_value()) {
            GameId id = opt_id.value();
            games.emplace(id, GameState(id, player_id, max_pawn, pawn_row, timeout));
            pending_game_id = id;
        }
    }
}

// We can safely take the game using .at, because message with non-existing game_id
// is taken as invalid. Server is single threaded, so no race-conditions are possible.
void Server::handle_move_1(PlayerId player_id, GameId game_id, PawnIndex pawn) {
    GameState& game = games.at(game_id);
    game.make_single_move(pawn, player_id);
}

void Server::handle_move_2(PlayerId player_id, GameId game_id, PawnIndex pawn) {
    GameState& game = games.at(game_id);
    game.make_double_move(pawn, player_id);
}

void Server::handle_keep_alive(PlayerId player_id, GameId game_id) {
    GameState& game = games.at(game_id);
    game.keep_alive(player_id);
}

void Server::handle_give_up(PlayerId player_id, GameId game_id) {
    GameState& game = games.at(game_id);
    game.give_up(player_id);
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
