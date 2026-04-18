#include "Message.h"

MessageType Message::get_type() const { return msg_type; }

uint32_t Message::get_player_id() const { return player_id; }

uint32_t Message::get_game_id() const { return game_id; }

uint8_t Message::get_pawn() const { return pawn; }

void Message::set_type(MessageType t) { msg_type = t; }

void Message::set_player_id(PlayerId p) { player_id = p; }

void Message::set_game_id(GameId g) { game_id = g; }

void Message::set_pawn_idx(PawnIndex p) { pawn = p; }
