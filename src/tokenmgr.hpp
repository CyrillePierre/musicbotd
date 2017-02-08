#pragma once

#include <string>

class TokenMgr {
	TokenMgr() = default;
	~TokenMgr() = default;

	TokenMgr(TokenMgr const&) = delete;
	TokenMgr(TokenMgr&&) = delete;
public:
	static TokenMgr &instance() { 
		static TokenMgr _instance;
		return _instance;
	}

	bool isValid(std::string const&token) const;
};
