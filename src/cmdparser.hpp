#pragma once

class Player;

class CmdParser {
    Player & _player;

public:
    CmdParser(Player & player) : _player{player} {}
    void apply(std::string const & cmd);
};
