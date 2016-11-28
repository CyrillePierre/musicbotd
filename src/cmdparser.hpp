#pragma once

#include <sstream>
#include "ytnamesolver.hpp"

class Player;

class CmdParser {
    Player & _player;
    YtNameSolver _yt;

public:
    CmdParser(Player & player) : _player{player} {}
    void apply(std::string const & cmd);

private:
    void add(std::istringstream & iss);
};
