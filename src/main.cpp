#include <iostream>
#include "net/server.hpp"
#include "player.hpp"
#include "cmdparser.hpp"

void printList(Player::PlayListView && list) {
    for (auto & it : list)
        std::cout << "  - (" << it->id() << ") " << it->title() << std::endl;
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
            std::string res{parser.apply(line)};
            if (!res.empty()) {
                res.push_back('\n');
                client.write(res.c_str(), res.size());
            }
            printList(player.list());
        }
    });

    std::cin.get();
    server.disconnect();

    return 0;
}
