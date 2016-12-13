#include <iomanip>
#include <cstdlib>
#include "cmdparserapi.hpp"
#include "player.hpp"
#include "ytnamesolver.hpp"
#include "config.hpp"
#include "nlohmann/json.hpp"

namespace elog = ese::log;

CmdParserAPI::CmdParserAPI(Player & player) : CmdParserBase{player} {
    _lg.prefix("cmdParserAPI: ");
}

std::string CmdParserAPI::add(std::istringstream & iss) {
    std::string id;
    iss >> id;
    nlohmann::json json;

    if (id.size() < cfg::ytIdSize) {
        std::string err{"Invalid video ID : '" + id + "'"};
        _lg(elog::warn) << err;
        json["error"] = err;
    } else {
        try {
            id = id.substr(id.size() - cfg::ytIdSize);
            std::string title(_yt.getVideoTitle(cfg::ytUrlPrefix + id));
            _lg(elog::dbg) << "video title = \"" << title << '"';
            _player.add(id, title);
            json["id"] = id;
            json["title"] = title;
        }
        catch (UnknownVideo const & e) {
            json["error"] = std::string{e.what()};
        }
    }

    return json.dump() + "\n";
}

std::string CmdParserAPI::list(std::istringstream & iss) {
    std::size_t nbLines;
    Player::PlayListView const * plv;
    if (iss >> nbLines) plv = &_player.list(nbLines);
    else plv = &_player.list();

    nlohmann::json json = nlohmann::json::array();

    int i = 0;
    for (auto const & it : *plv) {
        nlohmann::json item = nlohmann::json::object();
        item["index"] = i++;
        item["id"] = it->id();
        item["title"] = it->title();
        _lg(elog::trace) << "list += [" << it->id() << "] " << it->title();
        json += item;
    }
    return json.dump() + "\n";
}

std::string CmdParserAPI::rm(std::istringstream & iss) {
    nlohmann::json json;
    std::string id;
    if (iss >> id) {
        char * end;
        std::size_t index = std::strtoul(id.c_str(), &end, 10);
        if (!*end) {
            auto wm = _player.remove(index);
            if (wm) json["id"] = id;
        }
        else if (id.size() == cfg::ytIdSize){
            auto wm = _player.remove(id);
            if (wm) json["id"] = id;
        } else {
					json["error"] = "remove failed";
					_lg(elog::warn) << "remove failed (id = " << id << ')';
				}
		}
    else {
        json["error"] = "remove failed";
        _lg(elog::warn) << "Remove failed: parsing error";
    }
    return json.dump() + "\n";
}

std::string CmdParserAPI::clear(std::istringstream &) {
    nlohmann::json json;
    _player.clear();
    json["message"] = "clear";
    return json.dump() + "\n";
}

std::string CmdParserAPI::next(std::istringstream &) {
    nlohmann::json json;
    _player.next();
    json["message"] = "next";
    return json.dump() + "\n";
}

std::string CmdParserAPI::pause(std::istringstream &) {
    nlohmann::json json;
    _player.togglePause();
    json["message"] = "pause";
    json["playing"] = !_player.isPaused();
    return json.dump() + "\n";
}

std::string CmdParserAPI::volume(std::istringstream & iss) {
    Player::Volume value;
    nlohmann::json json;

    if (iss >> value)
        json["volume"] = _player.incrVolume(value);
    else
        json["volume"] = _player.volume();
    return json.dump() + "\n";
}

std::string CmdParserAPI::progress(std::istringstream & iss) {
    nlohmann::json json;
    json["current"] = _player.timePos();
    json["total"] = _player.duration();
    return json.dump() + "\n";
}

std::string CmdParserAPI::current(std::istringstream &) {
		nlohmann::json json;
		if (_player.hasCurrent()) {
				WebMusic wm{_player.current()};
				json["id"] = wm.id();
				json["title"] = wm.title();
		} else
				json["message"] = "No current music";
		return json.dump() + "\n";
}

std::string CmdParserAPI::state(std::istringstream &) {
		nlohmann::json json;
		json["hasCurrent"] = _player.hasCurrent();
		json["paused"] = _player.isPaused();
        return json.dump() + "\n";
}

std::string CmdParserAPI::random(std::istringstream &) {
    nlohmann::json json;
    auto wm = _player.addRandom();
    if (wm) {
        json["id"] = wm->id();
        json["title"] = wm->title();
    } else
        json["error"] = "Archive is empty";

    return json.dump() + "\n";
}
