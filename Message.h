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
public:
    Message(MessageType type, uint32_t player_id, uint32_t game_id, uint8_t pawn);

    [[nodiscard]] MessageType get_type() const;

    [[nodiscard]] uint32_t get_player_id() const;

    [[nodiscard]] uint32_t get_game_id() const;

    [[nodiscard]] uint8_t get_pawn() const;

private:
    MessageType msg_type;
    uint32_t player_id;
    uint32_t game_id;
    uint8_t pawn;
};



#endif //KAYLES_MESSAGE_H
