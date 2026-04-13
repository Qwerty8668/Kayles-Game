#include "GameState.h"

#include <climits>

GameState::GameState(const uint32_t id, const uint32_t a_id, const uint8_t max_pawn, const std::vector<std::byte>& pawns)
    : game_id(id), player_a_id(a_id), player_b_id(0), status(GameStatus::WAITING_FOR_OPPONENT), max_pawn(max_pawn), pawn_row(pawns) {

}

bool GameState::add_player(const uint32_t b_id) {
    if (player_b_id != 0) {
        return false;
    }
    player_b_id = b_id;
    status = GameStatus::TURN_A;
    return true;
}

bool GameState::is_player_turn(const uint32_t player_id) const {
    if (status != GameStatus::TURN_A && status != GameStatus::TURN_B) return false;
    if (status == GameStatus::TURN_A && player_id != player_a_id) return false;
    if (status == GameStatus::TURN_B && player_id != player_b_id) return false;
    return true;
}

bool GameState::is_pawn(const uint32_t index) const {
    size_t array_index = index / CHAR_BIT;
    size_t bit_index = index % CHAR_BIT;
    std::byte bit_mask = static_cast<std::byte>(1) << bit_index;
    return ((pawn_row[array_index] & bit_mask) == bit_mask);
}

void GameState::remove_pawn(const uint32_t index) {
    size_t array_index = index / CHAR_BIT;
    size_t bit_index = index % CHAR_BIT;
    std::byte bit_mask = static_cast<std::byte>(1) << bit_index;
    pawn_row[array_index] &= ~bit_mask;
}

void GameState::switch_turn() {
    if (status == GameStatus::TURN_A) {
        status = GameStatus::TURN_B;
    } else if (status == GameStatus::TURN_B) {
        status = GameStatus::TURN_A;
    }
}

void GameState::check_win_conditions() {
    size_t array_size = pawn_row.size();
    bool win = true;
    for (size_t i = 0; win && i < array_size; ++i) {
        if (pawn_row[i] != static_cast<std::byte>(0)) {
            win = false;
        }
    }
    if (win) {
        if (status == GameStatus::TURN_A) {
            status = GameStatus::WIN_A;
        } else if (status == GameStatus::TURN_B) {
            status = GameStatus::WIN_B;
        }
    }
}


bool GameState::make_single_move(const uint8_t index, const uint32_t player_id) {
    if (!is_player_turn(player_id)) return false;
    if (index > max_pawn) return false;
    if (!is_pawn(index)) return false;

    remove_pawn(index);

    check_win_conditions();
    switch_turn();
    return true;
}

bool GameState::make_double_move(const uint8_t index, const uint32_t player_id) {
    if (!is_player_turn(player_id)) return false;
    if (index + 1 > max_pawn) return false;
    if (!is_pawn(index)) return false;
    if (!is_pawn(index + 1)) return false;

    remove_pawn(index);
    remove_pawn(index + 1);

    check_win_conditions();
    switch_turn();
    return true;
}
