#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <bits/getopt_core.h>

#include "Client.h"
#include "common.h"
#include "err.h"
#include "Message.h"

uint32_t safe_parse_uint(const std::string& str) {
    if (str.empty()) fatal("Couldn't parse message.");;

    for (char c : str) {
        if (!std::isdigit(c)) {
            fatal("Couldn't parse message.");
        }
    }

    size_t pos = 0;
    uint32_t val = std::stoul(str, &pos);
    if (pos != str.length()) {
        fatal("Couldn't parse message.");
    }

    return val;
}

Message read_message(const std::string& input) {
    std::vector<std::string> tokens;
    std::stringstream ss(input);
    std::string token;

    while (std::getline(ss, token, '/')) {
        tokens.push_back(token);
    }

    if (tokens.empty()) fatal("Couldn't parse message.");

    Message msg;
    uint32_t type_val = safe_parse_uint(tokens[0]);
    if (type_val > 4) {
        fatal("Couldn't parse message.");
    }
    msg.msg_type = static_cast<MessageType>(type_val);

    uint32_t raw_player_id = 0, raw_game_id = 0, raw_pawn = 0;

    switch (msg.msg_type) {
        case MessageType::MSG_JOIN:
            if (tokens.size() != 2) fatal("Couldn't parse message.");;

            raw_player_id = safe_parse_uint(tokens[1]);
            break;

        case MessageType::MSG_MOVE_1:
        case MessageType::MSG_MOVE_2:
            if (tokens.size() != 4) fatal("Couldn't parse message.");;

            raw_player_id = safe_parse_uint(tokens[1]);
            raw_game_id = safe_parse_uint(tokens[2]);
            raw_pawn = safe_parse_uint(tokens[3]);
            if (raw_game_id > MAX_GAME_ID) {
                fatal("Couldn't parse message.");
            }
            if (raw_pawn > MAX_PAWN_IDX) {
                fatal("Couldn't parse message.");
            }
            msg.game_id = static_cast<GameId>(raw_game_id);
            msg.pawn = static_cast<PawnIndex>(raw_pawn);

            break;

        case MessageType::MSG_KEEP_ALIVE:
        case MessageType::MSG_GIVE_UP:
            if (tokens.size() != 3) fatal("Couldn't parse message.");

            raw_player_id = safe_parse_uint(tokens[1]);
            raw_game_id = safe_parse_uint(tokens[2]);
            if (raw_game_id > MAX_GAME_ID) {
                fatal("Couldn't parse message.");
            }
            msg.game_id = static_cast<GameId>(raw_game_id);
            break;
    }

    if (raw_player_id == 0 || raw_player_id > MAX_PLAYER_ID) {
        fatal("Couldn't parse message.");
    }
    msg.player_id = static_cast<PlayerId>(raw_player_id);
    return msg;
}

int main(int argc, char* argv[]) {
    std::string address;
    Message message;
    uint16_t port = -1;
    std::chrono::seconds timeout;

    bool has_a = false, has_p = false, has_m = false, has_t = false;

    int opt;
    while ((opt = getopt(argc, argv, "a:p:m:t:")) != -1) {
        switch (opt) {
            case 'a':
                address = optarg;
                has_a = true;
                break;
            case 'p':
                port = read_port(optarg);
                has_p = true;
                break;
            case 'm':
                message = read_message(optarg);
                has_m = true;
                break;
            case 't':
                timeout = read_timeout(optarg);
                has_t = true;
                break;
            case '?':
            default:
                fatal("Unknown argument");
        }
    }

    if (port == 0) {
        fatal("Port can't be 0.");
    }

    if (!has_a || !has_p || !has_m || !has_t) {
        fatal("Missing required arguments");
    }

    Client client = Client(address, port, message, timeout);

    client.run();
}