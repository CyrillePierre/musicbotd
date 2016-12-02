#pragma once

#include "logstream.hpp"

namespace log {

class Config {
    LogStream<char> _ls;
    int             _lvl;
    bool			_timeEnabled;

public:
    Config() : _ls{std::cout}, _lvl{1}, _timeEnabled{false} {}
    LogStream<char>::Stream & stream() { return _ls.stream(); }
    int logLevel() const { return _lvl; }
    void logLevel(int lvl) { _lvl = lvl; }
    void timeEnabled(bool b) { _timeEnabled = b; }
    bool timeEnabled() const { return _timeEnabled; }
};

inline Config & cfg() {
    static auto c = std::make_unique<Config>();
    return *c;
}

}
