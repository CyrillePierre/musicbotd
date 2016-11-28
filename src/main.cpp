#include <iostream>
#include "net/server.hpp"
#include "player.hpp"
#include "cmdparser.hpp"

void printList(Player::PlayListView && list) {
    for (auto & it : list)
        std::cout << "  - (" << it->id << ") " << it->name << std::endl;
}

int main() {
    Player player;

    net::Server server{1337};
    server.connect();
    server.asyncAcceptLoop([&player] (net::Client const & client) {
        char buf[128];
        CmdParser parser{player};

        while (std::size_t nbRead = client.read(buf, 128)) {
            buf[nbRead - 1] = 0;
            std::cout << "[Client] " << buf << std::endl;
            std::string line(buf, nbRead);
            parser.apply(line);
            printList(player.list());
        }
    });

    std::cin.get();
    server.disconnect();

    return 0;
}
