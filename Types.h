#ifndef KAYLES_TYPES_H
#define KAYLES_TYPES_H
#include <limits>
#include <cstdint>

using PlayerId = uint32_t;
using GameId = uint32_t;
using PawnIndex = uint8_t;

constexpr GameId MAX_GAME_ID = std::numeric_limits<GameId>::max();

constexpr size_t BUFFER_SIZE = 1024;

#endif //KAYLES_TYPES_H
