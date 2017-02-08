#pragma once

#include <string>
#include <vector>

class TokenMgr {
	TokenMgr();
	~TokenMgr() {}

	TokenMgr(TokenMgr const&) = delete;
	TokenMgr(TokenMgr&&) = delete;

	std::vector<std::string> _tokens;
public:
	static TokenMgr &instance() { 
		static TokenMgr _instance;
		return _instance;
	}

	bool isValid(std::string const&token) const;
};
