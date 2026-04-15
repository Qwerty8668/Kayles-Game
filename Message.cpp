#include "Message.h"


Message::Message(MessageType type, uint32_t player_id, uint32_t game_id, uint8_t pawn)
    : msg_type(type), player_id(player_id), game_id(game_id), pawn(pawn) {

}

MessageType Message::get_type() const { return msg_type; }

uint32_t Message::get_player_id() const { return player_id; }

uint32_t Message::get_game_id() const { return game_id; }

uint8_t Message::get_pawn() const { return pawn; }