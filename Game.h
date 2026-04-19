#ifndef KAYLES_GAME_H
#define KAYLES_GAME_H
#include <array>
#include <chrono>
#include <cstddef>
#include <queue>

#include "Types.h"

enum class GameStatus : uint8_t {
    WAITING_FOR_OPPONENT = 0,
    TURN_A = 1,
    TURN_B = 2,
    WIN_A = 3,
    WIN_B = 4
};

class Game {
public:
    Game(GameId id, PlayerId a_id, PawnIndex max_pawn, const std::array<std::byte, 32>& pawns,  std::chrono::seconds timeout);

    bool make_single_move(PawnIndex index, PlayerId player_id);

    bool make_double_move(PawnIndex index, PlayerId player_id);

    void add_player(PlayerId b_id);

    bool give_up(PlayerId player_id);

    void keep_alive(PlayerId player_id);

    bool has_player(PlayerId player_id) const;

    bool check_timeout();

    [[nodiscard]] GameId get_id() const;

    [[nodiscard]] PlayerId get_a_id() const;

    [[nodiscard]] PlayerId get_b_id() const;

    [[nodiscard]] GameStatus get_status() const;

    [[nodiscard]] PawnIndex get_max_pawn() const;

    [[nodiscard]] std::array<std::byte, 32> get_pawns() const;

private:
    GameId game_id;
    PlayerId player_a_id;
    PlayerId player_b_id = 0;
    GameStatus status = GameStatus::WAITING_FOR_OPPONENT;
    PawnIndex  max_pawn;
    std::array<std::byte, 32> pawn_row;
    std::chrono::steady_clock::time_point last_seen_a;
    std::chrono::steady_clock::time_point last_seen_b;
    std::chrono::seconds timeout;

    bool is_player_turn(PlayerId player_id) const;

    bool is_pawn(PawnIndex index) const;

    void remove_pawn(PawnIndex index);

    void check_win_conditions();

    void switch_turn();

    void update_last_seen(PlayerId player_id);

    static std::pair<size_t, std::byte> get_byte_index_and_mask(PawnIndex index);
};

#endif //KAYLES_GAME_H
