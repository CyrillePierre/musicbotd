#pragma once

#include "logstream.hpp"

namespace log {

class Config {
    LogStream<char> _ls          = std::cout;
    int             _lvl         = 1;
    bool			_timeEnabled = false;
    std::string		_timeFormat  = "%F %T";

public:
    LogStream<char>::Stream & stream() { return _ls.stream(); }
    template <class T>
    void stream(T && t) { _ls = LogStream<char>{std::forward<T>(t)}; }

    int logLevel() const { return _lvl; }
    void logLevel(int lvl) { _lvl = lvl; }
    void timeEnabled(bool b) { _timeEnabled = b; }
    bool timeEnabled() const { return _timeEnabled; }
    void timeFormat(std::string const & s) { _timeFormat = s; }
    std::string const & timeFormat() const { return _timeFormat; }
};

inline Config & cfg() {
    static auto c = std::make_unique<Config>();
    return *c;
}

}