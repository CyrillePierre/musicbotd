#include <iomanip>
#include <cstdlib>
#include "cmdparser.hpp"
#include "player.hpp"
#include "ytnamesolver.hpp"
#include "config.hpp"
#include <iostream>

std::string CmdParser::apply(std::string const & line) {
    std::istringstream iss(line);
    std::string cmd;

    iss >> cmd;

    if (cmd == "add")   return add(iss);
    if (cmd == "list")  return list(iss);
    if (cmd == "rm")    return rm(iss);
    if (cmd == "clear") return clear(iss);
    return "";
}

std::string CmdParser::add(std::istringstream & iss) {
    std::string id;
    iss >> id;

    try {
        id = id.substr(id.size() - cfg::ytIdSize);
        std::string title(_yt.getVideoTitle(cfg::ytUrlPrefix + id));
        _player.add(id, title);
        return "Adding: " + title + "\n";
    }
    catch (UnknownVideo const & e) {
        return std::string{e.what()} + "\n";
    }
}

std::string CmdParser::list(std::istringstream &) {
    std::ostringstream oss;
    oss << "Actual playlist:" << std::endl;

    int i = 0;
    for (auto const & it : _player.list()) {
        oss << std::setw(3) << i++ << ". [" << it->id();
        oss << "] " << it->title() << std::endl;
    }
    return oss.str();
}

std::string CmdParser::rm(std::istringstream & iss) {
    std::string id;
    if (iss >> id) {
        char * end;
        std::size_t index = std::strtoul(id.c_str(), &end, 10);
        if (!*end) {
            auto wm = _player.remove(index);
            if (wm) return "Removing: " + wm->title() + "\n";
        }
        else if (id.size() == cfg::ytIdSize){
            auto wm = _player.remove(id);
            if (wm) return "Removing: " + wm->title() + "\n";
        }
    }
    return "Remove failed.\n";
}

std::string CmdParser::clear(std::istringstream &) {
    _player.clear();
    return "Playlist cleared.\n";
}
