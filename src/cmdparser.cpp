#include "cmdparser.hpp"
#include "player.hpp"
#include "ytnamesolver.hpp"

void CmdParser::apply(std::string const & line) {
    std::istringstream iss(line);
    std::string cmd;

    iss >> cmd;

    if (cmd == "add") add(iss);
}

void CmdParser::add(std::istringstream & iss) {
    std::string param;
    iss >> param;
    std::string title(_yt.getVideoTitle(param));
    _player.add(param, title);
}
