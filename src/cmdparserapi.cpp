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

std::string CmdParserAPI::error(std::string const & msg) const {
	nlohmann::json json;
	json["error"] = msg;
	return json.dump();
}

std::string CmdParserAPI::add(std::istream & iss) {
	std::string id, errmsg;
	iss >> id;

	if (id.size() < cfg::ytIdSize) {
		std::string err{"Invalid video ID : '" + id + "'"};
		_lg(elog::warn) << err;
		errmsg = err;
	} else {
		try {
			id = id.substr(id.size() - cfg::ytIdSize);
			std::string title(_yt.getVideoTitle(cfg::ytUrlPrefix + id));
			_lg(elog::dbg) << "video title = \"" << title << '"';
			WebMusic wm{id, title};
			if (_archive) _archive->add(wm);
			if (_player.add(wm))
				return "";
			errmsg = "The playlist is full.";
		}
		catch (UnknownVideo const & e) {
			errmsg = std::string{e.what()};
		}
	}

	return error(errmsg) + "\n";
}

std::string CmdParserAPI::list(std::istream & iss) {
	std::size_t nbLines;
	Player::PlayListView const * plv;
	if (iss >> nbLines) plv = &_player.list(nbLines);
	else plv = &_player.list();

	nlohmann::json json;
	json["event"] = -1;
	json["value"] = nlohmann::json::array();

	int i{};
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

std::string CmdParserAPI::rm(std::istream & iss) {
	std::string id, errmsg;
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
			errmsg = "remove failed";
			_lg(elog::warn) << "remove failed (id = " << id << ')';
		}
	}
	else {
		errmsg = "remove failed";
		_lg(elog::warn) << "Remove failed: parsing error";
	}
	return error(errmsg) + "\n";
}

std::string CmdParserAPI::clear(std::istream &) {
	_player.clear();
	return "";
}

std::string CmdParserAPI::next(std::istream &) {
	_player.next();
	return "";
}

std::string CmdParserAPI::pause(std::istream &) {
	_player.togglePause();
	return "";
}

std::string CmdParserAPI::volume(std::istream & iss) {
	nlohmann::json json;
	Player::Volume value;

	json["event"] = 3;
	if (iss >> value) {
		_player.incrVolume(value);
		return "";
	} else
		json["value"] = _player.volume();
	return json.dump() + "\n";
}

std::string CmdParserAPI::progress(std::istream & iss) {
	nlohmann::json json;
	json["event"] = -2;
	json["value"]["current"] = _player.timePos();
	json["value"]["total"] = _player.duration();
	return json.dump() + "\n";
}

std::string CmdParserAPI::current(std::istream &) {
	nlohmann::json json = nlohmann::json::object();
	json["event"] = -3;
	if(_player.hasCurrent()) {
		auto const current = _player.current();
		json["value"]["id"] = current.id();
		json["value"]["title"] = current.title();
	} else
		json["no_current"] = "";
	return json.dump() + "\n";
}

std::string CmdParserAPI::state(std::istream &) {
	nlohmann::json json;
	json["event"] = 4;
	json["value"] = _player.isPaused();
	return json.dump() + "\n";
}

std::string CmdParserAPI::random(std::istream &) {
	bool ok = false;
	if(_archive && !_archive->empty()) ok = _player.add(_archive->random());
	else                               ok = _player.addRandom();
	if(ok) return "";
	return error("The playlist is full.") + "\n";
}

std::string CmdParserAPI::pl(std::istream & iss) {
	std::string fn;
	iss >> fn;
	if(_archive) _archivemgr.unload(std::move(_archive));
	_archive = _archivemgr.load(fn);
	if (_archive) {
		nlohmann::json json;
		json["event"] = 10;
		json["value"] = fn;
		return json.dump() + "\n";
	}
	return error("Invalid playlist name") + "\n";
}

std::string CmdParserAPI::plcur(std::istream &) {
	nlohmann::json json;
	json["event"] = 11;
	if(_archive) json["value"] = _archive->name();
	else         json["value"] = "~";
	return json.dump() + "\n";
}

std::string CmdParserAPI::plquit(std::istream &) {
	nlohmann::json json;
	json["event"] = 19;
	if(_archive) _archivemgr.unload(std::move(_archive));
	_archive.reset();
	return json.dump() + "\n";
}

std::string CmdParserAPI::pllist(std::istream & iss) {
	auto playlists = _archivemgr.list();
	nlohmann::json json;
	json["event"] = 12;
	json["value"] = nlohmann::json::array();
	for (auto const & elem : playlists)
		json["value"] += elem;
	return json.dump() + "\n";
}

std::string CmdParserAPI::subscribe(std::istream &) {
	_player.subscribe({reinterpret_cast<std::size_t>(this),
		[&]{
			std::istringstream dummy;
			random(dummy);
		}
	});
	return "\n";
}

std::string CmdParserAPI::unsubscribe(std::istream &) {
	_player.unsubscribe({reinterpret_cast<std::size_t>(this), []{}});
	return "\n";
}

std::string CmdParserAPI::auth(std::istream & iss) {
	std::string token;
	iss >> token;
	if(!(_auth = TokenMgr::instance().isValid(token)))
		return error("Authentication failed") + "\n";
	return "";
}
