#include <iostream>
#include "net/server.hpp"
#include "player.hpp"
#include "cmdparser.hpp"

void printList(Player::PlayListView const & list) {
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
            try {
                buf[--nbRead] = 0;
                std::string line(buf);
                std::cout << "[Client] " << line << std::endl;
                std::string res{parser.apply(line)};
                if (!res.empty()) client.write(res.c_str(), res.size());
            }
            catch (std::exception const & e) {
                std::cerr << e.what() << std::endl;
            }
        }
    });

    std::cin.get();
    server.disconnect();

    return 0;
}
