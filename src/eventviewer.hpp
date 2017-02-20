#pragma once
#include <functional>
#include <array>
#include <log/log.hpp>
#include "player.hpp"
#include <iostream>
#include "nlohmann/json.hpp"

namespace view {

using EventWriter    = std::function<std::string(util::Any)>;
using EventWriterAPI = std::function<nlohmann::json(util::Any)>;

std::array<EventWriter, 6> const eventWriters {
	[] (util::Any a) { return "Adding: " + a.cast<WebMusic>().title(); },
	[] (util::Any a) { return "Removing: " + a.cast<WebMusic>().title(); },
	[] (util::Any a) { return "Playing: " + a.cast<WebMusic>().title(); },
	[] (util::Any a) { return "New volume: " + std::to_string(a.cast<double>()) + "%"; },
	[] (util::Any a) { return a.cast<bool>() ? "Player paused" : "Player resumed."; },
	[] (util::Any)	 { return "Playlist cleared."; },
};

std::array<EventWriterAPI, 6> const eventWritersAPI {
	[] (util::Any a) {
		nlohmann::json json;
		json["id"] = a.cast<WebMusic>().id();
		json["title"] = a.cast<WebMusic>().title();
		return json;
	},
	[] (util::Any a) {
		nlohmann::json json;
		json["id"] = a.cast<WebMusic>().id();
		json["title"] = a.cast<WebMusic>().title();
		return json;
	},
	[] (util::Any a) {
		nlohmann::json json;
		json["id"] = a.cast<WebMusic>().id();
		json["title"] = a.cast<WebMusic>().title();
		return json;
	},
	[] (util::Any a) {
		nlohmann::json json;
		json[""] = a.cast<double>();
		return json;
	},
	[] (util::Any a) {
		nlohmann::json json;
		json[""] = a.cast<bool>();
		return json;
	},
	[] (util::Any)	 { return nlohmann::json{}; }
};

std::string eventFormat(PlayerEvt evt, util::Any a) {
	return eventWriters[evt](std::move(a));
}

std::string eventFormatAPI(PlayerEvt evt, util::Any a) {
	nlohmann::json json;
	json["event"] = evt;
	nlohmann::json value = eventWritersAPI[evt](std::move(a));
	if(value.count("")) json["value"] = value[""];
	else								json["value"] = value;
	return json.dump()+"\n";
}

}
