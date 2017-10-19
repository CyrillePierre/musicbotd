#pragma once

#include "cmdparserbase.hpp"

class CmdParserAPI: public CmdParserBase<CmdParserAPI> {
public:
	enum Event: int {
		Search = -7, Unsubscribe = -6, Subscribe = -5, Current = -3, Progress = -2, List = -1,
		Add = 0, Remove = 1, CurrentChanged = 2, Volume = 3, State = 4, Clear = 5, TimePos = 5,
		PlayListEnter = 10, PlayListCurrent = 11, PlayListList = 12, PlayListQuit = 19,
	};
	
public:
	CmdParserAPI(Player & player, ArchiveMgr & archivemgr);

private:
	std::string error(std::string const & msg) const;

	std::string add(std::istream & iss);
	std::string auth(std::istream & iss);
	std::string clear(std::istream & iss);
	std::string current(std::istream & iss);
	std::string list(std::istream & iss);
	std::string next(std::istream & iss);
	std::string pause(std::istream & iss);
	std::string play(std::istream & iss);
	std::string pl(std::istream & iss);
	std::string plcur(std::istream & iss);
	std::string pllist(std::istream & iss);
	std::string plquit(std::istream & iss);
	std::string plrm(std::istream & iss);
	std::string progress(std::istream & iss);
	std::string random(std::istream & iss);
	std::string rm(std::istream & iss);
	std::string search(std::istream & iss);
	std::string state(std::istream & iss);
	std::string subscribe(std::istream & iss);
	std::string unsubscribe(std::istream & iss);
	std::string volume(std::istream & iss);
	std::string move(std::istream & iss);

	friend CmdParserBase<CmdParserAPI>;
};
