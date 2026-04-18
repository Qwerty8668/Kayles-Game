

#include "Server.h"

int main() {

    std::array<std::byte, 32> pawns{};

    std::ranges::fill(pawns, static_cast<std::byte>(255));

    std::string ip = "127.0.0.1";

    Server server = Server(pawns, 255, ip, 1234, std::chrono::seconds(10));

    server.run();

    return 0;
}
