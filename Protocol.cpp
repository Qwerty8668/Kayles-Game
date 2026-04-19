#include "Protocol.h"

#include <cmath>
#include <cstring>
#include <netinet/in.h>

#include "GameState.h"
#include "WrongMessage.h"

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

    PawnIndex extract_pawn(const std::byte *ptr) {
        PawnIndex pawn_idx;
        memcpy(&pawn_idx, ptr, sizeof(PawnIndex));
        return pawn_idx;
    }

    GameStatus extract_game_status(const std::byte *ptr) {
        GameStatus gs{};
        memcpy(&gs, ptr, sizeof(GameStatus));
        return gs;
    }

    std::variant<Message, uint8_t> try_deserialize_join(std::span<const std::byte> buff) {
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
        msg.msg_type = type;
        msg.player_id = p_id;

        return msg;
    }

    std::variant<Message, uint8_t> try_deserialize_move(std::span<const std::byte> buff) {
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

        PawnIndex pwn_idx = extract_pawn(&buff[pos]);

        Message msg;
        msg.msg_type = type;
        msg.player_id = p_id;
        msg.game_id = g_id;
        msg.pawn = pwn_idx;

        return msg;
    }

    std::variant<Message, uint8_t> try_deserialize_status(std::span<const std::byte> buff) {
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
        msg.msg_type = type;
        msg.player_id = p_id;
        msg.game_id = g_id;

        return msg;
    }

    void insert_buffer_32(std::span<std::byte> buff, size_t &pos, uint32_t value) {
        uint32_t net = htonl(value);
        if (pos + sizeof(uint32_t) > buff.size()) {
            return;
        }
        std::memcpy(&buff[pos], &net, sizeof(uint32_t));
        pos += sizeof(uint32_t);
    }

    void insert_buffer_8(std::span<std::byte> buff, size_t &pos, uint8_t value) {
        if (pos + sizeof(uint8_t) > buff.size()) return;

        buff[pos++] = static_cast<std::byte>(value);
    }

    std::optional<GameState> try_deserialize_game_state(
        std::span<const std::byte> buff) {
        size_t min_size = sizeof(GameId) + 2 * sizeof(PlayerId) + sizeof(GameStatus) + sizeof(
                              PawnIndex);
        if (buff.size() < min_size) {
            return std::nullopt;
        }

        size_t pos = 0;

        GameId g_id = extract_game_id(&buff[pos]);
        pos += sizeof(GameId);

        PlayerId p_a_id = extract_player_id(&buff[pos]);
        pos += sizeof(PlayerId);

        PlayerId p_b_id = extract_player_id(&buff[pos]);
        pos += sizeof(PlayerId);

        GameStatus gs = extract_game_status(&buff[pos]);
        pos += sizeof(GameStatus);

        PawnIndex pwn_max = extract_pawn(&buff[pos]);
        pos += sizeof(PawnIndex);

        size_t array_size = (pwn_max / 8) + 1;

        if (buff.size() < pos + array_size) {
            return std::nullopt;
        }

        std::array<std::byte, 32> pawn_row{};

        for (size_t i = 0; i < array_size; i++) {
            pawn_row[i] = static_cast<std::byte>(buff[pos + i]);
        }

        GameState state{};
        state.game_id = g_id;
        state.player_a_id = p_a_id;
        state.player_b_id = p_b_id;
        state.status = gs;
        state.max_pawn = pwn_max;
        state.pawn_row = pawn_row;
        return state;
    }

    std::optional<WrongMessage> try_deserialize_wrong_msg(
        std::span<const std::byte> buff) {
        if (buff.size() != WRNG_MSG_BYTES_SIZE + 2) {
            return std::nullopt;
        }

        std::array<std::byte, WRNG_MSG_BYTES_SIZE> bytes{};

        for (size_t i = 0; i < WRNG_MSG_BYTES_SIZE; i++) {
            bytes[i] = static_cast<std::byte>(buff[i]);
        }

        uint8_t status = ERR_STATUS;

        auto error_idx = static_cast<std::uint8_t>(buff[WRNG_MSG_BYTES_SIZE + 1]);

        WrongMessage msg{};
        msg.first_bytes = bytes;
        msg.error_idx = error_idx;
        msg.status = status;
        return msg;
    }

    size_t serialize_join(const Message &msg, std::span<std::byte> buff) {
        size_t pos = 0;
        insert_buffer_8(buff, pos, static_cast<uint8_t>(msg.msg_type));
        insert_buffer_32(buff, pos, msg.player_id);
        return pos;
    }

    size_t serialize_move(const Message &msg, std::span<std::byte> buff) {
        size_t pos = 0;
        insert_buffer_8(buff, pos, static_cast<uint8_t>(msg.msg_type));
        insert_buffer_32(buff, pos, msg.player_id);
        insert_buffer_32(buff, pos, msg.game_id);
        insert_buffer_8(buff, pos, msg.pawn);
        return pos;
    }

    size_t serialize_status(const Message &msg, std::span<std::byte> buff) {
        size_t pos = 0;
        insert_buffer_8(buff, pos, static_cast<uint8_t>(msg.msg_type));
        insert_buffer_32(buff, pos, msg.player_id);
        insert_buffer_32(buff, pos, msg.game_id);
        return pos;
    }
}

namespace Protocol {
    std::variant<Message, uint8_t> try_deserialize_request(std::span<const std::byte> buff) {
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

    std::optional<std::variant<GameState, WrongMessage> > try_deserialize_response(
        std::span<const std::byte> buff) {
        // 13th byte is the status.
        if (buff.size() < WRNG_MSG_BYTES_SIZE + 1) {
            return std::nullopt;
        }

        uint8_t status;
        memcpy(&status, &buff[WRNG_MSG_BYTES_SIZE], sizeof(status));
        if (status < 5) {
            return try_deserialize_game_state(buff);
        }
        if (status == ERR_STATUS) {
            return try_deserialize_wrong_msg(buff);
        }
        return std::nullopt;
    }

    size_t serialize_request(const Message &msg, std::span<std::byte> buff) {
        switch (msg.msg_type) {
            case MessageType::MSG_JOIN:
                return serialize_join(msg, buff);
            case MessageType::MSG_MOVE_1:
            case MessageType::MSG_MOVE_2:
                return serialize_move(msg, buff);
            case MessageType::MSG_KEEP_ALIVE:
            case MessageType::MSG_GIVE_UP:
                return serialize_status(msg, buff);
        }
        return 0;
    }

    size_t serialize_game_state(const Game &game, std::span<std::byte> buff) {
        size_t pos = 0;
        insert_buffer_32(buff, pos, game.get_id());
        insert_buffer_32(buff, pos, game.get_a_id());
        insert_buffer_32(buff, pos, game.get_b_id());
        insert_buffer_8(buff, pos, static_cast<uint8_t>(game.get_status()));
        insert_buffer_8(buff, pos, game.get_max_pawn());

        std::array<std::byte, 32> pawns = game.get_pawns();
        size_t size = (game.get_max_pawn() / 8) + 1;
        for (size_t i = 0; i < size; i++) {
            insert_buffer_8(buff, pos, static_cast<uint8_t>(pawns[i]));
        }

        return pos;
    }

    size_t serialize_wrong_msg(std::span<const std::byte> packet,
                               uint8_t err_idx, std::span<std::byte> buff) {
        if (buff.size() < WRNG_MSG_BYTES_SIZE + 2) {
            return 0;
        }

        size_t copy_len = std::min<size_t>(packet.size(), WRNG_MSG_BYTES_SIZE);
        if (copy_len > 0) {
            std::memcpy(buff.data(), packet.data(), copy_len);
        }

        if (copy_len < WRNG_MSG_BYTES_SIZE) {
            std::memset(buff.data() + copy_len, 0, WRNG_MSG_BYTES_SIZE - copy_len);
        }

        buff[WRNG_MSG_BYTES_SIZE] = static_cast<std::byte>(ERR_STATUS);

        buff[WRNG_MSG_BYTES_SIZE + 1] = static_cast<std::byte>(err_idx);

        return WRNG_MSG_BYTES_SIZE + 2;
    }
}
