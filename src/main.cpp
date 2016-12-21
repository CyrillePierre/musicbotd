#include <iostream>
#include <net/server.hpp>
#include <log/log.hpp>
#include <functional>
#include "player.hpp"
#include "cmdparser.hpp"
#include "cmdparserapi.hpp"
#include "eventviewer.hpp"

namespace elog = ese::log;

template<typename Parser>
void applyFunction(Player & player, net::Client const & client) {
	elog::Logger lg;
	lg.prefix(std::string{"client "} + std::to_string(client.id()) + ": ");
	lg << "connected";

	char buf[128];
	Parser parser{player};

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
}

int main() {
	using namespace std::placeholders;

    elog::cfg().logLevel(elog::msg);
    elog::cfg().timeEnabled(true);
    elog::cfg().stream("/var/log/musicbotd.log");
    elog::Logger l;

	int port = 1937, portAPI = 1938;

    Archive archive{"/var/lib/musicbotd/archive"};
    Player player{archive};
    net::Server server{port}, serverAPI{portAPI};

	l << "server port (TCP): " << port;
	l << "API server port (TCP): " << portAPI;
	l << "connecting server";
	server.connect();
	serverAPI.connect();

	server.asyncAcceptLoop(std::bind(applyFunction<CmdParser>, std::ref(player), _1));
	serverAPI.asyncAcceptLoop(std::bind(applyFunction<CmdParserAPI>, std::ref(player), _1));

	l << "starting player";
	player.setEventHandler([&] (PlayerEvt evt, util::Any any) {
		std::string msg = view::eventFormat(evt, any);
		l << "server writeAll: '" << msg << "'";
		msg.push_back('\n');
		server.writeAll(msg.c_str(), msg.size());
		
		std::string msgAPI = view::eventFormatAPI(evt, any);
		serverAPI.writeAll(msgAPI.c_str(), msgAPI.size());
	});

	player.start();

//    std::cin.get();
	for (;;) std::this_thread::sleep_for(std::chrono::seconds{100});

	l << "disconnecting server";
	server.disconnect();
	serverAPI.disconnect();

	return 0;
}
