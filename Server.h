#ifndef KAYLES_SERVER_H
#define KAYLES_SERVER_H
#include <array>
#include <map>
#include <optional>
#include <string>
#include <netinet/in.h>

#include "GameState.h"
#include "Message.h"


class Server {
public:
    Server(std::array<std::byte, 32> &pawn_row, PawnIndex max_pawn, std::string &ip_address,
           uint16_t port, std::chrono::seconds timeout);

    void run();

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

    int init_socket() const;

    static std::pair<std::vector<uint8_t>, sockaddr> receive_packet(int socket_fd);

    void check_timeouts();

    std::optional<uint8_t> validate_packet(std::vector<uint8_t> &packet);

    GameState *handle_message(Message &msg);

    void send_game_state(struct sockaddr client, GameState *game);

    void send_wrong_msg(struct sockaddr client, std::vector<uint8_t> &packet, uint8_t err_idx);

    GameState *handle_join(PlayerId player_id);

    GameState *handle_move_1(PlayerId player_id, GameId game_id, PawnIndex pawn);

    GameState *handle_move_2(PlayerId player_id, GameId game_id, PawnIndex pawn);

    GameState *handle_keep_alive(PlayerId player_id, GameId game_id);

    GameState *handle_give_up(PlayerId player_id, GameId game_id);

    std::optional<GameId> generate_id();
};


#endif //KAYLES_SERVER_H
