#include "Protocol.h"

#include <cmath>
#include <cstring>
#include <math.h>
#include <netinet/in.h>

namespace {
    MessageType extract_type(const std::byte *ptr) {
        MessageType type;
        memcpy(&type, ptr, sizeof(MessageType));
        return type;
    }

    PlayerId extract_player_id(const std::byte *ptr) {
        PlayerId net_player_id;
        memcpy(&net_player_id, ptr, sizeof(PlayerId));
        PlayerId p_id = ntohl(net_player_id);
        return p_id;
    }

    GameId extract_game_id(const std::byte *ptr) {
        GameId net_game_id;
        memcpy(&net_game_id, ptr, sizeof(GameId));
        GameId g_id = ntohl(net_game_id);
        return g_id;
    }

    PawnIndex extract_pawn_idx(const std::byte *ptr) {
        PawnIndex pawn_idx;
        memcpy(&pawn_idx, ptr, sizeof(PawnIndex));
        return pawn_idx;
    }

    std::variant<Message, uint8_t> try_deserialize_join(const std::vector<std::byte> &buff) {
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

    std::variant<Message, uint8_t> try_deserialize_move(const std::vector<std::byte> &buff) {
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

    std::variant<Message, uint8_t> try_deserialize_status(const std::vector<std::byte> &buff) {
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

    void insert_buffer_32(std::vector<std::byte> &buff, uint32_t value) {
        GameId net = htonl(value);
        const auto *ptr = reinterpret_cast<const std::byte *>(&net);
        buff.insert(buff.end(), ptr, ptr + sizeof(uint32_t));
    }

    void insert_buffer_8(std::vector<std::byte> &buff, uint8_t value) {
        buff.push_back(static_cast<std::byte>(value));
    }
}

namespace Protocol {
    std::variant<Message, uint8_t> try_deserialize_request(const std::vector<std::byte> &buff) {
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

    std::vector<std::byte> serialize_game_state(const Game &game) {
        std::vector<std::byte> buff;

        insert_buffer_32(buff, game.get_id());
        insert_buffer_32(buff, game.get_a_id());
        insert_buffer_32(buff, game.get_b_id());
        insert_buffer_8(buff, static_cast<uint8_t>(game.get_status()));
        insert_buffer_8(buff, game.get_max_pawn());

        std::array<std::byte, 32> pawns = game.get_pawns();
        size_t size = (game.get_max_pawn() / 8) + 1;
        for (size_t i = 0; i < size; i++) {
            insert_buffer_8(buff, static_cast<uint8_t>(pawns[i]));
        }

        return buff;
    }

    std::vector<std::byte> serialize_wrong_msg(const std::vector<std::byte> &packet,
                                               uint8_t err_idx) {
        std::vector<std::byte> buff;

        size_t copy_len = std::min<size_t>(packet.size(), 12);

        buff.insert(buff.end(), packet.begin(),
                    packet.begin() + static_cast<std::ptrdiff_t>(copy_len));
        buff.insert(buff.end(), 12 - copy_len, std::byte{0});
        buff.push_back(static_cast<std::byte>(255));
        buff.push_back(static_cast<std::byte>(err_idx));

        return buff;
    }
}
