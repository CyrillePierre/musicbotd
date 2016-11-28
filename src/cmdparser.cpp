#include "cmdparser.hpp"
#include "player.hpp"
#include "ytnamesolver.hpp"
#include "config.hpp"

std::string CmdParser::apply(std::string const & line) {
    std::istringstream iss(line);
    std::string cmd;

    iss >> cmd;

    if (cmd == "add") return add(iss);
    return "";
}

std::string CmdParser::add(std::istringstream & iss) {
    std::string id;
    iss >> id;

    try {
        id = id.substr(id.size() - cfg::ytIdSize);
        std::string title(_yt.getVideoTitle(cfg::ytUrlPrefix + id));
        _player.add(id, title);
        return "Adding: " + title;
    }
    catch (UnknownVideo & e) {
        return e.what();
    }
}
