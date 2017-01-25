#include <iomanip>
#include <cstdlib>
#include "cmdparserapi.hpp"
#include "player.hpp"
#include "ytnamesolver.hpp"
#include "config.hpp"
#include "nlohmann/json.hpp"

namespace elog = ese::log;

CmdParserAPI::CmdParserAPI(Player & player, ArchiveMgr & archivemgr) : CmdParserBase{player, archivemgr} {
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
			WebMusic wm{id, title};
			if(_archive) _archive->add(wm);
			_player.add(wm);
			return "";
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

	nlohmann::json json;
	json["event"] = -1;
	json["value"] = nlohmann::json::array();

	int i = _player.hasCurrent();
	if(i) {
		auto const current = _player.current();
		auto const it = &current;
		nlohmann::json item = nlohmann::json::object();
		item["index"] = i++;
		item["id"] = it->id();
		item["title"] = it->title();
		_lg(elog::trace) << "list += [" << it->id() << "] " << it->title();
		json["value"] += item;
	}

	for (auto const & it : *plv) {
		nlohmann::json item = nlohmann::json::object();
		item["index"] = i++;
		item["id"] = it->id();
		item["title"] = it->title();
		_lg(elog::trace) << "list += [" << it->id() << "] " << it->title();
		json["value"] += item;
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
			_player.remove(index);
			return "";
		} else if (id.size() == cfg::ytIdSize) {
			_player.remove(id);
			return "";
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
	_player.clear();
	return "";
}

std::string CmdParserAPI::next(std::istringstream &) {
	_player.next();
	return "";
}

std::string CmdParserAPI::pause(std::istringstream &) {
	_player.togglePause();
	return "";
}

std::string CmdParserAPI::volume(std::istringstream & iss) {
	nlohmann::json json;
	Player::Volume value;

	json["event"] = 3;
	if (iss >> value) {
		_player.incrVolume(value);
		return "";
	} else
		json["value"] = std::to_string(_player.volume());
	return json.dump() + "\n";
}

std::string CmdParserAPI::progress(std::istringstream & iss) {
	nlohmann::json json;
	json["event"] = -2;
	json["value"]["current"] = _player.timePos();
	json["value"]["total"] = _player.duration();
	return json.dump() + "\n";
}

std::string CmdParserAPI::current(std::istringstream &) {
	return "";
}

std::string CmdParserAPI::state(std::istringstream &) {
	nlohmann::json json;
	json["event"] = 4;
	json["value"] = std::to_string(_player.isPaused());
	return json.dump() + "\n";
}

std::string CmdParserAPI::random(std::istringstream &) {
	if(_archive && !_archive->empty()) _player.add(_archive->random());
	else _player.addRandom();
	return "";
}

std::string CmdParserAPI::pl(std::istringstream & iss) {
	nlohmann::json json;
	std::string fn;
	iss >> fn;
	if(_archive) _archivemgr.unload(std::move(_archive));
	_archive = _archivemgr.load(fn);
    if (_archive) {
        json["event"] = 10;
        json["value"] = fn;
        return json.dump() + "\n";
    }
    json["error"] = "Invalid playlist name";
    return json.dump() + "\n";
}

std::string CmdParserAPI::plcur(std::istringstream &) {
	nlohmann::json json;
	json["event"] = 11;
	if(_archive) json["value"] = _archive->name();
	else         json["value"] = "~";
	return json.dump() + "\n";
}

std::string CmdParserAPI::plquit(std::istringstream &) {
	nlohmann::json json;
	json["event"] = 19;
	if(_archive) _archivemgr.unload(std::move(_archive));
	return json.dump() + "\n";
}

std::string CmdParserAPI::pllist(std::istringstream & iss) {
    auto playlists = _archivemgr.list();
    nlohmann::json json;
    json["value"] = nlohmann::json::array();
    for (auto const & elem : playlists)
        json["value"] += elem;
    return json.dump() + "\n";
}
