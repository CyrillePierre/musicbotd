#include <iostream>
#include <net/server.hpp>
#include <log/log.hpp>
#include "player.hpp"
#include "cmdparser.hpp"

int main() {
    log::cfg().logLevel(log::trace);
    log::Logger l;

    int port = 1937;

    Player player;
    net::Server server{port};

    l << "server port (TCP): " << port;
    l << "connecting server";
    server.connect();
    server.asyncAcceptLoop([&player, &server] (net::Client const & client) {
        log::Logger lg;
        lg.prefix(std::string{"client "} + std::to_string(client.id()) + ": ");
        lg << "connected";

        char buf[128];
        CmdParser parser{player};

        while (std::size_t nbRead = client.read(buf, 128)) {
            try {
                buf[--nbRead] = 0;
                std::string line(buf);
                lg << "cmd '" << line << "'";
                std::string res{parser.apply(line)};
                if (!res.empty()) {
                    lg << "server writeAll: '" << res << "'";
                    server.writeAll(res.c_str(), res.size());
                }
            }
            catch (std::exception const & e) {
                lg(log::err) << e.what();
            }
        }

        lg << "disconnected";
    });

    l << "starting player";
    player.start();

    std::cin.get();
    l << "disconnecting server";
    server.disconnect();

    return 0;
}
