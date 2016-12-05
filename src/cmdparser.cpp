#include <iomanip>
#include <cstdlib>
#include "cmdparser.hpp"
#include "player.hpp"
#include "ytnamesolver.hpp"
#include "config.hpp"

CmdParser::CmdParser(Player & player) : _player{player} {
    _lg.prefix("cmdParser: ");
}

std::string CmdParser::apply(std::string const & line) {
    std::istringstream iss(line);
    std::string cmd;

    iss >> cmd;

    if (cmd == "add")    return add(iss);
    if (cmd == "list")   return list(iss);
    if (cmd == "rm")     return rm(iss);
    if (cmd == "clear")  return clear(iss);
    if (cmd == "next")   return next(iss);
    if (cmd == "pause")  return pause(iss);
    if (cmd == "volume") return volume(iss);

    _lg(log::warn) << "unknown command '" << cmd << "'";
    return "unknown command '" + cmd + "'\n";
}

std::string CmdParser::add(std::istringstream & iss) {
    std::string id;
    iss >> id;

    if (id.size() < cfg::ytIdSize) {
        std::string err{"Invalid video ID : '" + id};
        _lg(log::warn) << err;
        return std::move(err) + "\n";
    }

    try {
        id = id.substr(id.size() - cfg::ytIdSize);
        std::string title(_yt.getVideoTitle(cfg::ytUrlPrefix + id));
        _lg(log::dbg) << "video title = \"" << title << '"';
        _player.add(id, title);
        return "";
    }
    catch (UnknownVideo const & e) {
        return std::string{e.what()} + "\n";
    }
}

std::string CmdParser::list(std::istringstream & iss) {
    std::size_t nbLines;
    Player::PlayListView const * plv;
    if (iss >> nbLines) plv = &_player.list(nbLines);
    else plv = &_player.list();

    std::ostringstream oss;
    oss << "Current playlist:" << std::endl;

    int i = 0;
    for (auto const & it : *plv) {
        oss << std::setw(3) << i++ << ". [" << it->id();
        oss << "] " << it->title() << std::endl;
        _lg(log::trace) << "list += [" << it->id() << "] " << it->title();
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
            if (wm) return "";
        }
        else if (id.size() == cfg::ytIdSize){
            auto wm = _player.remove(id);
            if (wm) return "";
        }
        _lg(log::warn) << "remove failed (id = " << id << ')';
    }
    else _lg(log::warn) << "Remove failed: parsing error";
    return "Remove failed.\n";
}

std::string CmdParser::clear(std::istringstream &) {
    _player.clear();
    return "";
}

std::string CmdParser::next(std::istringstream &) {
    _player.next();
    return "";
}

std::string CmdParser::pause(std::istringstream &) {
    _player.togglePause();
    return "";
}

std::string CmdParser::volume(std::istringstream & iss) {
    Player::Volume value;

    if (iss >> value) {
        Player::Volume vol = _player.incrVolume(value);
        return "";
    }
    return "Volume = " + std::to_string(_player.volume()) + "\n";
}
