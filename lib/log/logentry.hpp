#pragma once

#include "config.hpp"

namespace log {

class Logger;

class LogEntry {
    bool _token;
public:
    int  lvl;

    LogEntry(int lvl) : _token{true}, lvl{lvl} {}
    LogEntry(LogEntry && le) : _token{le._token}, lvl{le.lvl}
    { le._token = false; }
    ~LogEntry();
};

template <class T>
LogEntry operator << (LogEntry && le, T && t) {
    if (le.lvl >= cfg().logLevel())
        cfg().stream() << std::forward<T>(t);
    return std::move(le);
}

}
