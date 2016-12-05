#pragma once

#include <sstream>
#include <log/log.hpp>
#include "ytnamesolver.hpp"

class Player;

class CmdParser {
    Player & _player;
    YtNameSolver _yt;
    log::Logger _lg;

public:
    CmdParser(Player & player);
    std::string apply(std::string const & cmd);

private:
    std::string add(std::istringstream & iss);
    std::string list(std::istringstream & iss);
    std::string rm(std::istringstream & iss);
    std::string clear(std::istringstream & iss);
    std::string next(std::istringstream & iss);
    std::string pause(std::istringstream & iss);
};
