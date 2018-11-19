#include <iostream>
#include <net/server.hpp>
#include <log/log.hpp>
#include <functional>
#include <csignal>

#include "player.hpp"
#include "cmdparser.hpp"
#include "cmdparserapi.hpp"
#include "eventviewer.hpp"
#include "archivemgr.hpp"

#define MB_BASE_PATH	""

long const TIMEOUT = 30; // seconds

namespace elog = ese::log;

template<typename Parser, elog::LogLevel lglvl>
void applyFunction(Player & player, ArchiveMgr & archivemgr, net::Client const & client) {
	elog::Logger lg;
	lg.prefix(std::string{"client "} + std::to_string(client.id()) + ": ");
	lg << "connected";

	std::string buf;
	Parser parser{player, archivemgr};

	while (std::size_t nbRead = client.readUntil(buf, '\n')) {
		try {
			buf[nbRead] = 0;
			std::string line(buf);
            lg(lglvl) << "cmd '" << line << "'";
			std::string res{parser.apply(line)};
			if (!res.empty()) {
                lg(lglvl) << "write " << res.size() << " bytes";
				client.write(res.c_str(), res.size());
			}
		}
		catch (std::exception const & e) {
			lg(elog::err) << e.what();
		}
	}

	lg << "disconnected";
}

int main() try {
	using namespace std::placeholders;

	static volatile bool signalReceived = false;
	static std::condition_variable cv;

	struct sigaction action;
	action.sa_handler = [](int) { signalReceived = true; cv.notify_one(); };
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGTERM, &action, NULL);

	elog::cfg().logLevel(elog::trace);
	elog::cfg().timeEnabled(true);
	elog::Logger l;

	int port = 1937, portAPI = 1938;

	ArchiveMgr archivemgr{MB_BASE_PATH "pl"};
	Archive archive{MB_BASE_PATH "archive"};

	Player player{archive};
	{
		std::ifstream in{MB_BASE_PATH "musicbotd.save"};
		if(in) in >> player;
	}

	net::Server server{port, TIMEOUT}, serverAPI{portAPI, TIMEOUT};

	l << "server port (TCP): " << port;
	l << "API server port (TCP): " << portAPI;
	l << "connecting server";
	server.connect();
	serverAPI.connect();

	server.asyncAcceptLoop(std::bind(
		applyFunction<CmdParser, elog::msg>, std::ref(player), std::ref(archivemgr), _1), signalReceived);
	serverAPI.asyncAcceptLoop(std::bind(
		applyFunction<CmdParserAPI, elog::dbg>, std::ref(player), std::ref(archivemgr), _1), signalReceived);

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

	{
		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);
		cv.wait(lock, []{ return signalReceived; });

		std::ofstream of{MB_BASE_PATH "musicbotd.save"};
		of << player;
	}

	l << "disconnecting server";
	server.disconnect();
	serverAPI.disconnect();

	return 0;
}
catch (std::exception const & e) {
	elog::Logger l;
	l(elog::crit) << "Exception: " << e.what();
}
