#include <iomanip>
#include <cstdlib>
#include "cmdparser.hpp"
#include "player.hpp"
#include "ytnamesolver.hpp"
#include "config.hpp"

namespace elog = ese::log;

CmdParser::CmdParser(Player & player, ArchiveMgr & archivemgr) : CmdParserBase{player, archivemgr} {
    _lg.prefix("cmdParser: ");
}

std::string CmdParser::error(std::string const & msg) const {
	return msg;
}

std::string CmdParser::add(std::istringstream & iss) {
    std::string id;
    iss >> id;

    if (id.size() < cfg::ytIdSize) {
        std::string err{"Invalid video ID : '" + id};
        _lg(elog::warn) << err;
        return std::move(err) + "\n";
    }

    try {
        id = id.substr(id.size() - cfg::ytIdSize);
        std::string title(_yt.getVideoTitle(cfg::ytUrlPrefix + id));
        _lg(elog::dbg) << "video title = \"" << title << '"';
				WebMusic wm{id, title};
				if(_archive) _archive->add(wm);
        if (_player.add(wm))
            return "";
        return "The playlist is full.\n";
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
        _lg(elog::trace) << "list += [" << it->id() << "] " << it->title();
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
        _lg(elog::warn) << "remove failed (id = " << id << ')';
    }
    else _lg(elog::warn) << "Remove failed: parsing error";
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
        _player.incrVolume(value);
        return "";
    }
    return "Volume = " + std::to_string(_player.volume()) + "\n";
}

std::string CmdParser::progress(std::istringstream & iss) {
    double dur = _player.duration();
    double time = _player.timePos();
    std::ostringstream oss;
    oss << "Progress: " << time << "s / " << dur << "s (";
    oss << time*100/dur << "%)\n";
    return oss.str();
}

std::string CmdParser::current(std::istringstream &) {
    std::ostringstream oss;
    if (_player.hasCurrent()) {
        WebMusic wm{_player.current()};
        oss << "Current: [" << wm.id() << "] " << wm.title() << '\n';
    }
    else oss << "No current music.\n";
    return oss.str();
}

std::string CmdParser::state(std::istringstream &) {
    std::ostringstream oss;
    oss << std::boolalpha;
    oss << "States: hasCurrent=" << _player.hasCurrent();
    oss << ", isPaused=" << _player.isPaused() << '\n';
    return oss.str();
}

std::string CmdParser::random(std::istringstream &) {
		bool ok = false;
    if(_archive && !_archive->empty()) ok = _player.add(_archive->random());
    else                               ok = _player.addRandom();
		if(ok)	return "";
    return "The playlist is full.\n";
}

std::string CmdParser::pl(std::istringstream & iss) {
	std::string fn;
	iss >> fn;
	if(_archive) _archivemgr.unload(std::move(_archive));
	_archive = _archivemgr.load(fn);
    if (_archive) return "Entering playlist: "+fn+"\n";
    return "Invalid playlist name.\n";
}

std::string CmdParser::plcur(std::istringstream & iss) {
	if(_archive) return _archive->name() + "\n";
	return "~\n";
}

std::string CmdParser::plquit(std::istringstream &) {
	if(_archive) _archivemgr.unload(std::move(_archive));
	_archive.reset();
	return "Entering default playlist\n";
}

std::string CmdParser::pllist(std::istringstream &) {
    auto playlists = _archivemgr.list();
    std::ostringstream oss;
    oss << "Available playlists:" << std::endl;
    for (auto const & elem : playlists)
        oss << "  - " << elem << std::endl;
    return oss.str();
}

std::string CmdParser::subscribe(std::istringstream &) {
	bool ok = _player.subscribe({reinterpret_cast<std::size_t>(this),
		[&]{
			std::istringstream dummy;
			random(dummy);
		}
	});
	if(!ok)	return "You are already subscribed\n";
	return "You are now subscribed to automatically insert new musics\n";
}

std::string CmdParser::unsubscribe(std::istringstream &) {
	bool ok = _player.unsubscribe({reinterpret_cast<std::size_t>(this), []{}});
	if(!ok)	return "You were not subscribed\n";
	return "You are now unsubscribed from automatically inserting new musics\n";
}

std::string CmdParser::auth(std::istringstream & iss) {
	std::string token;
	iss >> token;
	_auth = TokenMgr::instance().isValid(token);
	return _auth? "Authentication success\n":"Authentication failed\n";
}
