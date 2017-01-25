#pragma once

#include "cmdparserbase.hpp"

class CmdParser: public CmdParserBase<CmdParser> {
public:
    CmdParser(Player & player, ArchiveMgr & archivemgr);

private:
    std::string add(std::istringstream & iss);
    std::string list(std::istringstream & iss);
    std::string rm(std::istringstream & iss);
    std::string clear(std::istringstream & iss);
    std::string next(std::istringstream & iss);
    std::string pause(std::istringstream & iss);
    std::string volume(std::istringstream & iss);
    std::string progress(std::istringstream & iss);
    std::string current(std::istringstream & iss);
    std::string state(std::istringstream & iss);
    std::string random(std::istringstream & iss);
    std::string pl(std::istringstream & iss);
    std::string plcur(std::istringstream & iss);
    std::string plquit(std::istringstream & iss);

    friend CmdParserBase<CmdParser>;
};
