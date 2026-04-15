#include "GameState.h"
#include "Types.h"
#include <climits>
#include <functional>

GameState::GameState(const GameId id, const PlayerId a_id, const PawnIndex max_pawn, const std::array<std::byte, 32>& pawns,  std::chrono::seconds timeout)
    : game_id(id), player_a_id(a_id), max_pawn(max_pawn), pawn_row(pawns), timeout(timeout) {
    last_seen_a = std::chrono::steady_clock::now();
}

bool GameState::make_single_move(const PawnIndex index, const PlayerId player_id) {
    update_last_seen(player_id);
    if (!is_player_turn(player_id)) return false;
    if (index > max_pawn) return false;
    if (!is_pawn(index)) return false;

    remove_pawn(index);

    check_win_conditions();
    switch_turn();
    return true;
}

bool GameState::make_double_move(const PawnIndex index, const PlayerId player_id) {
    update_last_seen(player_id);
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

void GameState::add_player(const PlayerId b_id) {
    player_b_id = b_id;
    status = GameStatus::TURN_B;
    update_last_seen(b_id);
}

bool GameState::give_up(PlayerId player_id) {
    update_last_seen(player_id);
    // Player can give up only when it is his turn.
    if (!is_player_turn(player_id)) return false;

    if (status == GameStatus::TURN_A) {
        status = GameStatus::WIN_B;
    } else if (status == GameStatus::TURN_B) {
        status = GameStatus::WIN_A;
    }
    return true;
}

void GameState::keep_alive(PlayerId player_id) {
    update_last_seen(player_id);
}

/* Returns true if game can be deleted. False otherwise */
bool GameState::check_timeout() {
    auto now = std::chrono::steady_clock::now();

    if (status == GameStatus::WAITING_FOR_OPPONENT) {
        if (now - last_seen_a >= timeout) {
            return true;
        }
    } else if (status == GameStatus::TURN_A || status == GameStatus::TURN_B) {
        bool a_timedout = (now - last_seen_a >= timeout);
        bool b_timedout = (now - last_seen_b >= timeout);

        if (a_timedout && b_timedout) {
            return true;
        } else if (a_timedout) {
            status = GameStatus::WIN_B;
        } else if (b_timedout) {
            status = GameStatus::WIN_A;
        }
    } else if (status == GameStatus::WIN_A || status == GameStatus::WIN_B) {
        auto last_seen = std::max(last_seen_a, last_seen_b);
        if (now - last_seen >= timeout) {
            return true;
        }
    }
    return false;
}

bool GameState::is_player_turn(const PlayerId player_id) const {
    if (status != GameStatus::TURN_A && status != GameStatus::TURN_B) return false;
    if (status == GameStatus::TURN_A && player_id != player_a_id) return false;
    if (status == GameStatus::TURN_B && player_id != player_b_id) return false;
    return true;
}

bool GameState::is_pawn(const PawnIndex index) {
    auto [array_index, bit_mask] = get_byte_index_and_mask(index);
    return ((pawn_row[array_index] & bit_mask) == bit_mask);
}

void GameState::remove_pawn(const PawnIndex index) {
    auto [array_index, bit_mask] = get_byte_index_and_mask(index);
    pawn_row[array_index] &= ~bit_mask;
}

void GameState::check_win_conditions() {
    // Check if every pawn is removed.
    static constexpr std::array<std::byte, 32> empty_row{};

    if (pawn_row == empty_row) {
        if (status == GameStatus::TURN_A) {
            status = GameStatus::WIN_A;
        } else if (status == GameStatus::TURN_B) {
            status = GameStatus::WIN_B;
        }
    }
}

void GameState::switch_turn() {
    if (status == GameStatus::TURN_A) {
        status = GameStatus::TURN_B;
    } else if (status == GameStatus::TURN_B) {
        status = GameStatus::TURN_A;
    }
}

void GameState::update_last_seen(PlayerId player_id) {
    if (player_id == player_a_id) {
        last_seen_a = std::chrono::steady_clock::now();
    } else if (player_id == player_b_id) {
        last_seen_b = std::chrono::steady_clock::now();
    }
}

std::pair<size_t, std::byte> GameState::get_byte_index_and_mask(PawnIndex index) {
    // Pawn numbered 0 is the most significant bit.
    size_t array_index = index / CHAR_BIT;
    size_t bit_index = CHAR_BIT - 1 - (index % CHAR_BIT);
    std::byte bit_mask = static_cast<std::byte>(1) << bit_index;
    return {array_index, bit_mask};
}
