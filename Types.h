#ifndef KAYLES_TYPES_H
#define KAYLES_TYPES_H
#include <limits>
#include <cstdint>

using PlayerId = uint32_t;
using GameId = uint32_t;
using PawnIndex = uint8_t;

constexpr GameId MAX_PLAYER_ID = std::numeric_limits<PlayerId>::max();
constexpr GameId MAX_GAME_ID = std::numeric_limits<GameId>::max();
constexpr GameId MAX_PAWN_IDX = std::numeric_limits<PawnIndex>::max();

constexpr size_t BUFFER_SIZE = 1024;
constexpr size_t WRNG_MSG_BYTES_SIZE = 12;
constexpr uint8_t ERR_STATUS = 255;

#endif //KAYLES_TYPES_H
