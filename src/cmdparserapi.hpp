#pragma once

#include "cmdparserbase.hpp"

class CmdParserAPI: public CmdParserBase<CmdParserAPI> {
public:
    CmdParserAPI(Player & player);

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

		friend CmdParserBase<CmdParserAPI>;
};