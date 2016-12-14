#pragma once
#include <functional>
#include <array>
#include <log/log.hpp>
#include "player.hpp"
#include <iostream>
#include "nlohmann/json.hpp"

namespace view {

using EventWriter =  std::function<std::string(util::Any)>;

std::array<EventWriter, 6> const eventWriters {
	[] (util::Any a) { return "Adding: " + a.cast<WebMusic>().title(); },
	[] (util::Any a) { return "Removing: " + a.cast<WebMusic>().title(); },
	[] (util::Any a) { return "Playing: " + a.cast<WebMusic>().title(); },
	[] (util::Any a) { return "New volume: " + std::to_string(a.cast<double>()) + "%"; },
	[] (util::Any a) { return a.cast<bool>() ? "Player paused" : "Player resumed."; },
	[] (util::Any)	 { return "Playlist cleared."; },
};

std::array<EventWriter, 6> const eventWritersAPI {
	[] (util::Any a)	{ return a.cast<WebMusic>().id(); },
	[] (util::Any a)	{ return a.cast<WebMusic>().id(); },
	[] (util::Any a)	{ return a.cast<WebMusic>().id(); },
	[] (util::Any a)	{ return std::to_string(a.cast<double>()); },
	[] (util::Any a)	{ return std::to_string(a.cast<bool>()); },
	[] (util::Any)		{ return ""; }
};

std::string eventFormat(PlayerEvt evt, util::Any a) {
	return eventWriters[evt](std::move(a));
}

std::string eventFormatAPI(PlayerEvt evt, util::Any a) {
	nlohmann::json json;
	json["event"] = evt;
	json["value"] = eventWritersAPI[evt](std::move(a));
	return json.dump()+"\n";
}

}
