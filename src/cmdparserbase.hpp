#pragma once

#include <sstream>
#include <log/log.hpp>
#include "ytnamesolver.hpp"
#include "player.hpp"

template<typename Impl>
class CmdParserBase {
    Impl *_this;
protected:
    Player & _player;
    YtNameSolver _yt;
    ese::log::Logger _lg;

public:
    CmdParserBase(Player & player): _this{static_cast<Impl*>(this)}, _player{player} {}

    std::string apply(std::string const & line) {
        namespace elog = ese::log;

        std::istringstream iss{line};
        std::string cmd;

        iss >> cmd;

        if (cmd == "add")		return _this->add(iss);
        if (cmd == "list")		return _this->list(iss);
        if (cmd == "rm")		return _this->rm(iss);
        if (cmd == "clear")		return _this->clear(iss);
        if (cmd == "next")		return _this->next(iss);
        if (cmd == "pause")		return _this->pause(iss);
        if (cmd == "volume")	return _this->volume(iss);
        if (cmd == "progress")	return _this->progress(iss);
        if (cmd == "current")	return _this->current(iss);
        if (cmd == "state")		return _this->state(iss);
        if (cmd == "random")	return _this->random(iss);

        _lg(elog::warn) << "unknown command '" << cmd << "'";
        return "unknown command '" + cmd + "'\n";
    }
};
