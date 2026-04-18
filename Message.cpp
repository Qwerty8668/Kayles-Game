#include "Message.h"

#include <cstring>
#include <netinet/in.h>

std::variant<Message, uint8_t> Message::try_deserialize(const std::vector<std::byte> &buff) {
    if (buff.empty()) {
        return static_cast<uint8_t>(0);
    }

    switch (extract_type(&buff[0])) {
        case MessageType::MSG_JOIN:
            return try_deserialize_join(buff);
        case MessageType::MSG_MOVE_1:
        case MessageType::MSG_MOVE_2:
            return try_deserialize_move(buff);
        case MessageType::MSG_KEEP_ALIVE:
        case MessageType::MSG_GIVE_UP:
            return try_deserialize_status(buff);
        default:
            return static_cast<uint8_t>(0);
    }
}

MessageType Message::get_type() const { return msg_type; }

uint32_t Message::get_player_id() const { return player_id; }

uint32_t Message::get_game_id() const { return game_id; }

uint8_t Message::get_pawn() const { return pawn; }

std::variant<Message, uint8_t> Message::try_deserialize_join(const std::vector<std::byte> &buff) {
    size_t size = sizeof(MessageType) + sizeof(PlayerId);
    if (buff.size() < size) {
        return static_cast<uint8_t>(buff.size());
    }
    if (buff.size() > size) {
        return static_cast<uint8_t>(size);
    }

    size_t pos = 0;

    MessageType type = extract_type(&buff[pos]);
    pos += sizeof(MessageType);

    PlayerId p_id = extract_player_id(&buff[pos]);

    Message msg;
    msg.set_type(type);
    msg.set_player_id(p_id);

    return msg;
}

std::variant<Message, uint8_t> Message::try_deserialize_move(const std::vector<std::byte> &buff) {
    size_t size = sizeof(MessageType) + sizeof(PlayerId) + sizeof(GameId) + sizeof(PawnIndex);
    if (buff.size() < size) {
        return static_cast<uint8_t>(buff.size());
    }
    if (buff.size() > size) {
        return static_cast<uint8_t>(size);
    }

    size_t pos = 0;

    MessageType type = extract_type(&buff[pos]);
    pos += sizeof(MessageType);

    PlayerId p_id = extract_player_id(&buff[pos]);
    pos += sizeof(PlayerId);

    GameId g_id = extract_game_id(&buff[pos]);
    pos += sizeof(GameId);

    PawnIndex pwn_idx = extract_pawn_idx(&buff[pos]);

    Message msg;
    msg.set_type(type);
    msg.set_player_id(p_id);
    msg.set_game_id(g_id);
    msg.set_pawn_idx(pwn_idx);

    return msg;
}

std::variant<Message, uint8_t> Message::try_deserialize_status(const std::vector<std::byte> &buff) {
    size_t size = sizeof(MessageType) + sizeof(PlayerId) + sizeof(GameId);
    if (buff.size() < size) {
        return static_cast<uint8_t>(buff.size());
    }
    if (buff.size() > size) {
        return static_cast<uint8_t>(size);
    }

    size_t pos = 0;

    MessageType type = extract_type(&buff[pos]);
    pos += sizeof(MessageType);

    PlayerId p_id = extract_player_id(&buff[pos]);
    pos += sizeof(PlayerId);

    GameId g_id = extract_game_id(&buff[pos]);

    Message msg;
    msg.set_type(type);
    msg.set_player_id(p_id);
    msg.set_game_id(g_id);

    return msg;
}

MessageType Message::extract_type(const std::byte *ptr) {
    MessageType type;
    memcpy(&type, ptr, sizeof(MessageType));
    return type;
}

PlayerId Message::extract_player_id(const std::byte *ptr) {
    PlayerId net_player_id;
    memcpy(&net_player_id, ptr, sizeof(PlayerId));
    PlayerId p_id = ntohl(net_player_id);
    return p_id;
}

GameId Message::extract_game_id(const std::byte *ptr) {
    GameId net_game_id;
    memcpy(&net_game_id, ptr, sizeof(GameId));
    GameId g_id = ntohl(net_game_id);
    return g_id;
}

PawnIndex Message::extract_pawn_idx(const std::byte *ptr) {
    PawnIndex pawn_idx;
    memcpy(&pawn_idx, ptr, sizeof(PawnIndex));
    return pawn_idx;
}

void Message::set_type(MessageType t) { msg_type = t; }
void Message::set_player_id(PlayerId p) { player_id = p; }
void Message::set_game_id(GameId g) { game_id = g; }
void Message::set_pawn_idx(PawnIndex p) { pawn = p; }
