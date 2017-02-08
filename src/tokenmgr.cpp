#include "tokenmgr.hpp"
#include <fstream>
#include <algorithm>
#include <vector>
#include <string>

bool TokenMgr::isValid(std::string const&token) const {
	std::ifstream f{"/etc/musicbotd/tokens"};
	std::vector<std::string> tokens;
	if(f) for(std::string line; std::getline(f, line); tokens.push_back(line));
	return std::find(tokens.begin(), tokens.end(), token) != tokens.end();
}
