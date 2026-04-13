#ifndef KAYLES_MESSAGE_H
#define KAYLES_MESSAGE_H
#include <cstdint>

enum class MessageType : uint8_t {
    MSG_JOIN = 0,
    MSG_MOVE_1 = 1,
    MSG_MOVE_2 = 2,
    MSG_KEEP_ALIVE = 3,
    MSG_GIVE_UP = 4
};

class Message {
private:
    MessageType msg_type;
    uint32_t player_id;
    uint32_t game_id;
    uint8_t pawn;
public:
    Message(MessageType msg_type, uint32_t player_id, uint32_t game_id, uint8_t pawn);
};



#endif //KAYLES_MESSAGE_H
