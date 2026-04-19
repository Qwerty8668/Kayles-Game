#ifndef KAYLES_PROTOCOL_H
#define KAYLES_PROTOCOL_H
#include <variant>
#include <optional>
#include <span>

#include "Game.h"
#include "GameState.h"
#include "Message.h"
#include "WrongMessage.h"


namespace Protocol {
    std::variant<Message, uint8_t> try_deserialize_request(std::span<const std::byte> buff);

    std::optional<std::variant<GameState, WrongMessage> > try_deserialize_response(
        std::span<const std::byte> buff);

    size_t serialize_request(const Message &msg, std::span<std::byte> buff);

    size_t serialize_game_state(const Game &game, std::span<std::byte> buff);

    size_t serialize_wrong_msg(std::span<const std::byte> packet,
                               uint8_t err_idx, std::span<std::byte> buff);
};


#endif //KAYLES_PROTOCOL_H
