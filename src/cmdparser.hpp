#pragma once

#include <sstream>
#include "ytnamesolver.hpp"

class Player;

class CmdParser {
    Player & _player;
    YtNameSolver _yt;

public:
    CmdParser(Player & player) : _player{player} {}
    std::string apply(std::string const & cmd);

private:
    std::string add(std::istringstream & iss);
    std::string list(std::istringstream & iss);
};
