#ifndef KAYLES_WRONGMESSAGE_H
#define KAYLES_WRONGMESSAGE_H
#include <array>
#include <cstdint>

class WrongMessage {
public:
    std::array<std::byte, 12> first_bytes;
    uint8_t status;
    uint8_t error_idx;
};

#endif //KAYLES_WRONGMESSAGE_H
