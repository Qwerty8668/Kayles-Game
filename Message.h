#ifndef KAYLES_MESSAGE_H
#define KAYLES_MESSAGE_H
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

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
    std::string serialize() const;

    static std::variant<Message, uint8_t> try_deserialize(const std::vector<std::byte>& buff);

    [[nodiscard]] MessageType get_type() const;

    [[nodiscard]] uint32_t get_player_id() const;

    [[nodiscard]] uint32_t get_game_id() const;

    [[nodiscard]] uint8_t get_pawn() const;

private:
    MessageType msg_type = MessageType::MSG_JOIN;
    PlayerId player_id = 0;
    GameId game_id = 0;
    PawnIndex pawn = 0;

    static std::variant<Message, uint8_t> try_deserialize_join(const std::vector<std::byte>& buff);

    static std::variant<Message, uint8_t> try_deserialize_move(const std::vector<std::byte>& buff);

    static std::variant<Message, uint8_t> try_deserialize_status(const std::vector<std::byte>& buff);

    static MessageType extract_type(const std::byte *ptr);

    static PlayerId extract_player_id(const std::byte *ptr);

    static GameId extract_game_id(const std::byte *ptr);

    static PawnIndex extract_pawn_idx(const std::byte *ptr);

    void set_type(MessageType t);

    void set_player_id(PlayerId p);

    void set_game_id(GameId g);

    void set_pawn_idx(PawnIndex p);
};


#endif //KAYLES_MESSAGE_H
