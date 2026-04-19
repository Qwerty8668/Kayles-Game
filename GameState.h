#ifndef KAYLES_GAMESTATE_H
#define KAYLES_GAMESTATE_H

#include "Game.h"
#include "Types.h"

class GameState {
public:
    GameId game_id;
    PlayerId player_a_id;
    PlayerId player_b_id;
    GameStatus status;
    PawnIndex max_pawn;
    std::array<std::byte, 32> pawn_row;
};



#endif //KAYLES_GAMESTATE_H
