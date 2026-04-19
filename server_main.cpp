#include <cstring>
#include <iostream>
#include <limits.h>
#include <bits/getopt_core.h>

#include "common.h"
#include "err.h"
#include "Server.h"

std::array<std::byte, 32> read_pawn_row(const std::string& input) {
    if (input.empty() || input.length() > 256) {
        fatal("Invalid pawn row");
    }

    if (input.front() != '1' || input.back() != '1') {
        fatal("Invalid pawn row");
    }

    std::array<std::byte, 32> arr{};

    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '1') {
            size_t byte_idx = i / CHAR_BIT;
            size_t bit_offset = CHAR_BIT - 1 - (i % CHAR_BIT);

            arr[byte_idx] |= static_cast<std::byte>(1 << bit_offset);

        } else if (input[i] != '0') {
            fatal("Invalid pawn row");
        }
    }

    return arr;
}

int main(int argc, char* argv[]) {
    std::string address;
    std::array<std::byte, 32> pawn_row{};
    uint8_t max_pawn = 0;
    uint16_t port = -1;
    std::chrono::seconds timeout;

    bool has_r = false, has_a = false, has_p = false, has_t = false;

    int opt;
    while ((opt = getopt(argc, argv, "r:a:p:t:")) != -1) {
        switch (opt) {
            case 'r':
                max_pawn = static_cast<uint8_t>(strlen(optarg) - 1);
                pawn_row = read_pawn_row(optarg);
                has_r = true;
                break;
            case 'a':
                address = optarg;
                has_a = true;
                break;
            case 'p':
                port = read_port(optarg);
                has_p = true;
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

    if (!has_r || !has_a || !has_p || !has_t) {
        fatal("Missing required arguments");
    }

    Server server = Server(pawn_row, max_pawn, address, port, timeout);

    server.run();

    return 0;
}
