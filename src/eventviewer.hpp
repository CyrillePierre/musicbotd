#pragma once
#include <functional>
#include <array>
#include <log/log.hpp>
#include "player.hpp"
#include <iostream>

namespace view {

using EventWriter =  std::function<std::string(util::Any)>;

std::array<EventWriter, 6> const eventWriters {
    [] (util::Any a) { return "Adding: " + a.cast<WebMusic>().title(); },
    [] (util::Any a) { return "Removing: " + a.cast<WebMusic>().title(); },
    [] (util::Any a) { return "Playing: " + a.cast<WebMusic>().title(); },
    [] (util::Any a) { return "New volume: " + std::to_string(a.cast<double>()) + "%"; },
    [] (util::Any a) { return a.cast<bool>() ? "Player paused" : "Player resumed."; },
    [] (util::Any)   { return "Playlist cleared."; },
};


std::string eventFormat(PlayerEvt evt, util::Any a) {
    return eventWriters[evt](std::move(a));
}

}
