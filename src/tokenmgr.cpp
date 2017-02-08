#include "tokenmgr.hpp"
#include <fstream>
#include <algorithm>

TokenMgr::TokenMgr() {
	std::ifstream f{"/etc/musicbotd/tokens"};
	if(f) for(std::string line; std::getline(f, line); _tokens.push_back(line));
}

bool TokenMgr::isValid(std::string const&token) const {
	return std::find(_tokens.begin(), _tokens.end(), token) != _tokens.end();
}
