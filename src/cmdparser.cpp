#include <sstream>
#include "cmdparser.hpp"
#include "player.hpp"

void CmdParser::apply(std::string const & line) {
    std::istringstream iss(line);
    std::string cmd;

    iss >> cmd;

    if (cmd == "add") {
        std::string param;
        iss >> param;
        _player.add(param, "");
    }
}
