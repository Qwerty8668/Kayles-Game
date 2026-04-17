#include "Message.h"


MessageType Message::get_type() const { return msg_type; }

uint32_t Message::get_player_id() const { return player_id; }

uint32_t Message::get_game_id() const { return game_id; }

uint8_t Message::get_pawn() const { return pawn; }
