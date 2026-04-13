#include "Message.h"


Message::Message(MessageType msg_type, uint32_t player_id, uint32_t game_id, uint8_t pawn)
    : msg_type(msg_type), player_id(player_id), game_id(game_id), pawn(pawn) {

}
