#ifndef KAYLES_PROTOCOL_H
#define KAYLES_PROTOCOL_H
#include <variant>
#include <optional>

#include "Game.h"
#include "GameState.h"
#include "Message.h"
#include "WrongMessage.h"


namespace Protocol {

    // Messages sent by Client to the Server

    std::variant<Message, uint8_t> try_deserialize_request(const std::vector<std::byte>& buff);

    std::vector<std::byte> serialize_request(const Message& request);

    // MSG_GAME_STATE

    std::vector<std::byte> serialize_game_state(const Game& game);

    std::vector<std::byte> serialize_wrong_msg(const std::vector<std::byte>& packet, uint8_t err_idx);

    std::optional<std::variant<GameState, WrongMessage> > try_deserialize_response(
        const std::vector<std::byte> &buff);

};



#endif //KAYLES_PROTOCOL_H
