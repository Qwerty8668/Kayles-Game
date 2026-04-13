#ifndef KAYLES_GAME_H
#define KAYLES_GAME_H
#include <cstddef>
#include <cstdint>
#include <vector>

enum class GameStatus : uint8_t {
    WAITING_FOR_OPPONENT = 0,
    TURN_A = 1,
    TURN_B = 2,
    WIN_A = 3,
    WIN_B = 4
};

class GameState {
private:
    uint32_t game_id;
    uint32_t player_a_id;
    uint32_t player_b_id;
    GameStatus  status;
    uint8_t  max_pawn;
    std::vector<std::byte> pawn_row;
    [[nodiscard]] bool is_player_turn(uint32_t player_id) const;
    [[nodiscard]] bool is_pawn(uint32_t index) const;

    void remove_pawn(uint32_t index);

    void switch_turn();

    void check_win_conditions();

public:
    GameState(uint32_t id, uint32_t a_id, uint8_t max_pawn, const std::vector<std::byte>& pawns);
    bool add_player(uint32_t b_id);
    bool make_single_move(uint8_t index, uint32_t player_id);
    bool make_double_move(uint8_t index, uint32_t player_id);




};

#endif //KAYLES_GAME_H
