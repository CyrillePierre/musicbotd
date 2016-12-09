#include <iostream>
#include <net/server.hpp>
#include <log/log.hpp>
#include "player.hpp"
#include "cmdparser.hpp"
#include "eventviewer.hpp"

namespace elog = ese::log;

int main() {
    elog::cfg().logLevel(elog::trace);
    elog::Logger l;

    int port = 1937;

    Player player;
    net::Server server{port};

    l << "server port (TCP): " << port;
    l << "connecting server";
    server.connect();
    server.asyncAcceptLoop([&player, &server] (net::Client const & client) {
        elog::Logger lg;
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
                    lg << "write " << res.size() << " bytes";
                    client.write(res.c_str(), res.size());
                }
            }
            catch (std::exception const & e) {
                lg(elog::err) << e.what();
            }
        }

        lg << "disconnected";
    });

    l << "starting player";
    player.setEventHandler([&server, &l] (PlayerEvt evt, util::Any any) {
        std::string msg = view::eventFormat(evt, std::move(any));
        l << "server writeAll: '" << msg << "'";
        msg.push_back('\n');
        server.writeAll(msg.c_str(), msg.size());
    });

    player.start();

    std::cin.get();
    l << "disconnecting server";
    server.disconnect();

    return 0;
}
