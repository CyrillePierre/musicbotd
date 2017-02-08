#pragma once

#include <sstream>
#include <log/log.hpp>
#include "ytnamesolver.hpp"
#include "player.hpp"
#include "archivemgr.hpp"
#include "archive.hpp"
#include "tokenmgr.hpp"

template<typename Impl>
class CmdParserBase {
	Impl *_this;
protected:
	Player & _player;
	ArchiveMgr & _archivemgr;
	YtNameSolver _yt;
	ese::log::Logger _lg;
	std::shared_ptr<Archive> _archive;

	bool _auth;

public:
	CmdParserBase(Player & player, ArchiveMgr & archivemgr):
		_this{static_cast<Impl*>(this)}, _player{player}, _archivemgr{archivemgr}, _auth{false} {}

	~CmdParserBase() {
		if(_archive) _archivemgr.unload(std::move(_archive));
	}

	std::string apply(std::string const & line) {
		namespace elog = ese::log;

		std::istringstream iss{line};
		std::string cmd;

		iss >> cmd;

    if (cmd == "add")       return _this->add(iss);
    if (cmd == "list")      return _this->list(iss);
    if (cmd == "rm")        return _this->rm(iss);
    if (cmd == "clear")     return _this->clear(iss);
    if (cmd == "next")      return _this->next(iss);
    if (cmd == "pause")     return _this->pause(iss);
    if (cmd == "volume")    return _this->volume(iss);
    if (cmd == "progress")  return _this->progress(iss);
    if (cmd == "current")   return _this->current(iss);
    if (cmd == "state")     return _this->state(iss);
    if (cmd == "random")    return _this->random(iss);
		if (cmd == "pl")        return _this->pl(iss);
		if (cmd == "plcur")     return _this->plcur(iss);
		if (cmd == "plquit")    return _this->plquit(iss);
		if (cmd == "help")      return help(iss);
    if (cmd == "pllist")    return _this->pllist(iss);
		if (cmd == "auth")      return _this->auth(iss);
		if (_auth) {
			if (cmd == "tts")       return [&]{ tts("fr-FR", iss); return ""; }();
			if (cmd == "ttsen")     return [&]{ tts("en-GB", iss); return ""; }();
		}

		_lg(elog::warn) << "unknown command '" << cmd << "'";
		return _this->error("unknown command '" + cmd + "'") + "\n";
	}

private:
    std::string help(std::istringstream &) {
        return R"#(Available commands:
 add (<id>|<url>)  Add a video ID or URL of youtube video in the playlist.
 rm (<index>|<id>) Remove a video by ID or index of youtube video in the playlist.
 next              Pass to the next music.
 vol <value>       Increment the volume with the corresponding <value> (in %).
 pause             Pause the current music.
 list [<N>]        List the <N> next videos (all videos if <N> is omitted).
 random            Randomly select a music in archive file.
 pl <name>         Enter in the playlist <name>.
 plcur             Display the current playlist.
 plquit            Enter the global playlist.
 pllist            List all existing playlists.
 clear             Remove all the playlist.
 state             Show the current states of the player.
 progress          Show the current position in the current music.
 current           Show the current video name.
 auth token        Authenticate with a specified token
 tts text          [auth] Text-to-Speech
 ttsen text        [auth] English Text-to-Speech
)#";
    }

		void tts(std::string const & lang, std::istringstream & iss) {
			std::string text, cmd;
			std::getline(iss, text);
			std::replace(text.begin(), text.end(), '"', '\'');
			cmd = "pico2wave -l"+lang+" -w/var/lib/musicbotd/tts.wav \""+text.substr(0, 256)+"\"";
			system(cmd.c_str());
		}
};
