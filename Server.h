#ifndef KAYLES_SERVER_H
#define KAYLES_SERVER_H
#include <array>
#include <map>
#include <optional>
#include <string>
#include <netinet/in.h>
#include <span>

#include "Game.h"
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
    std::map<GameId, Game> games;
    std::optional<GameId> pending_game_id;
    GameId next_game_id = 0;

    int init_socket() const;

    static size_t receive_packet(int socket_fd, std::span<std::byte> buffer,
                                 sockaddr_in &client_address);

    void check_timeouts();

    Game *handle_message(Message &msg);

    std::optional<uint8_t> validate_message(Message &msg);

    static void send_game_state(int sockfd, struct sockaddr_in client, const Game &game);

    static void send_wrong_msg(int sockfd, struct sockaddr_in client,
                               std::span<const std::byte> &packet, uint8_t err_idx);

    Game *handle_join(PlayerId player_id);

    Game *handle_move_1(PlayerId player_id, GameId game_id, PawnIndex pawn);

    Game *handle_move_2(PlayerId player_id, GameId game_id, PawnIndex pawn);

    Game *handle_keep_alive(PlayerId player_id, GameId game_id);

    Game *handle_give_up(PlayerId player_id, GameId game_id);

    static std::optional<uint8_t> validate_join(PlayerId player_id);

    std::optional<uint8_t> validate_args(PlayerId player_id, GameId game_id);

    std::optional<GameId> generate_id();
};


#endif //KAYLES_SERVER_H
