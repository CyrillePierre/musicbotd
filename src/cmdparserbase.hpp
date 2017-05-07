#pragma once

#include <sstream>
#include <log/log.hpp>
#include <unistd.h>
#include <sys/wait.h>
#include <iomanip>

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
		{
			std::istringstream dummy{};
			_this->unsubscribe(dummy);
		}
		if(_archive) _archivemgr.unload(std::move(_archive));
	}

	std::string apply(std::string const & line) {
		namespace elog = ese::log;

		std::istringstream iss{line};
		std::string cmd;

		iss >> cmd;

    if (cmd == "add")         return _this->add(iss);
		if (cmd == "auth")        return _this->auth(iss);
    if (cmd == "clear")       return _this->clear(iss);
    if (cmd == "current")     return _this->current(iss);
		if (cmd == "help")        return help(iss);
    if (cmd == "list")        return _this->list(iss);
    if (cmd == "next")        return _this->next(iss);
    if (cmd == "pause")       return _this->pause(iss);
		if (cmd == "pl")          return _this->pl(iss);
		if (cmd == "plcur")       return _this->plcur(iss);
    if (cmd == "pllist")      return _this->pllist(iss);
		if (cmd == "plquit")      return _this->plquit(iss);
    if (cmd == "progress")    return _this->progress(iss);
    if (cmd == "random")      return _this->random(iss);
    if (cmd == "rm")          return _this->rm(iss);
    if (cmd == "state")       return _this->state(iss);
		if (cmd == "subscribe")	  return _this->subscribe(iss);
		if (cmd == "unsubscribe") return _this->unsubscribe(iss);
    if (cmd == "volume")      return _this->volume(iss);
		if (_auth) {
			if (cmd == "time")			return [&]{
				std::time_t t = std::time(nullptr);
				std::tm tm = *std::localtime(&t);
				std::ostringstream oss;
				oss.imbue(std::locale("fr_FR.utf8"));
				oss << std::put_time(&tm, "Il est %R");
				speak("fr", oss.str());
				return "";
			}();
			if (cmd == "tts")       return [&]{ tts(iss); return ""; }();
		}

		_lg(elog::warn) << "unknown command '" << cmd << "'";
		return _this->error("unknown command '" + cmd + "'") + "\n";
	}

private:
    std::string help(std::istringstream &) {
        return R"#(Available commands:
 add (<id>|<url>)  Add a video ID or URL of youtube video in the playlist.
 auth token        Authenticate with a specified token
 clear             Remove all the playlist.
 current           Show the current video name.
 list [<N>]        List the <N> next videos (all videos if <N> is omitted).
 next              Pass to the next music.
 pause             Pause the current music.
 pl <name>         Enter in the playlist <name>.
 plcur             Display the current playlist.
 pllist            List all existing playlists.
 plquit            Enter the global playlist.
 progress          Show the current position in the current music.
 random            Randomly select a music in archive file.
 rm (<index>|<id>) Remove a video by ID or index of youtube video in the playlist.
 state             Show the current states of the player.
 subscribe         Subscribe to automatically insert musics
 unsubscribe       Unsubscribe to automatically insert musics
 vol <value>       Increment the volume with the corresponding <value> (in %).
 ----------------- Require authentification:
 time              Announce time (fr)
 tts lang text     Text-to-Speech
)#";
    }

		void speak(std::string lang, std::string text) {
			bool mustPause = !_player.isPaused();
			if(mustPause) _player.togglePause();

			int pid;
			if((pid = fork()) == 0) {
				execlp("simple_google_tts", "simple_google_tts", lang.c_str(), text.c_str(), NULL);
				exit(0);
			}
			waitpid(pid, NULL, 0);
			if(mustPause) _player.togglePause();
		}

		void tts(std::istringstream & iss) {
			std::string lang, text;
			iss >> lang;
			std::getline(iss, text);
			std::replace(text.begin(), text.end(), '"', '\'');
			text = "\""+text.substr(0, 256)+"\"";
			speak(lang, text);
		}
};
