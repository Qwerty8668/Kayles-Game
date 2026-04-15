#ifndef KAYLES_SERVER_H
#define KAYLES_SERVER_H
#include <array>
#include <map>
#include <optional>
#include <string>

#include "GameState.h"
#include "Message.h"


class Server {
public:
    Server(std::array<std::byte, 32>& pawn_row, PawnIndex max_pawn, std::string& ip_address, uint16_t port, std::chrono::seconds timeout);
    void run();

    void handle_join(PlayerId player_id);

    void handle_move_1(PlayerId player_id, GameId game_id, PawnIndex pawn);

    void handle_move_2(PlayerId player_id, GameId game_id, PawnIndex pawn);

    void handle_keep_alive(PlayerId player_id, GameId game_id);

    void handle_give_up(PlayerId player_id, GameId game_id);

    void check_timeouts();

    std::optional<GameId> generate_id();

    void handle_message(Message msg);

private:
    std::array<std::byte, 32> pawn_row;
    PawnIndex max_pawn;
    std::string ip_address;
    uint16_t port;
    const std::chrono::seconds timeout;
    std::queue<GameId> free_ids;
    std::map<GameId, GameState> games;
    std::optional<GameId> pending_game_id;
    GameId next_game_id = 0;

    void init_socket();

    Message receive_and_deserialize();
};



#endif //KAYLES_SERVER_H
