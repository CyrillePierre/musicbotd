#pragma once

#include "cmdparserbase.hpp"

class CmdParser: public CmdParserBase<CmdParser> {
public:
	CmdParser(Player & player, ArchiveMgr & archivemgr);

private:
	std::string error(std::string const & msg) const;

	std::string add(std::istream & iss);
	std::string auth(std::istream & iss);
	std::string clear(std::istream & iss);
	std::string current(std::istream & iss);
	std::string list(std::istream & iss);
	std::string next(std::istream & iss);
	std::string pause(std::istream & iss);
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

	friend CmdParserBase<CmdParser>;
};
