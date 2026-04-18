#ifndef KAYLES_PROTOCOL_H
#define KAYLES_PROTOCOL_H
#include <variant>

#include "GameState.h"
#include "Message.h"


namespace Protocol {

    // Messages sent by Client to the Server

    std::variant<Message, uint8_t> try_deserialize_request(const std::vector<std::byte>& buff);

    std::vector<std::byte> serialize_request(const Message& request);

    // MSG_GAME_STATE

    std::vector<std::byte> serialize_game_state(const GameState& game);

    GameState deserialize_game_state(const std::vector<std::byte>& buff);

    std::vector<std::byte> serialize_wrong_msg(const std::vector<std::byte>& packet, uint8_t err_idx);

};



#endif //KAYLES_PROTOCOL_H
