#ifndef KAYLES_MESSAGE_H
#define KAYLES_MESSAGE_H
#include <cstdint>
#include "Types.h"

enum class MessageType : uint8_t {
    MSG_JOIN = 0,
    MSG_MOVE_1 = 1,
    MSG_MOVE_2 = 2,
    MSG_KEEP_ALIVE = 3,
    MSG_GIVE_UP = 4
};

class Message {
public:
    MessageType msg_type = MessageType::MSG_JOIN;
    PlayerId player_id = 0;
    GameId game_id = 0;
    PawnIndex pawn = 0;
};


#endif //KAYLES_MESSAGE_H
