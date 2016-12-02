#pragma once

#include "config.hpp"

namespace log {

class Logger;

struct LogEntry {
    Logger * logger;
    int      lvl;

    LogEntry(Logger * l, int lvl) : logger{l}, lvl{lvl} {}
    LogEntry(LogEntry && le) : logger{le.logger}, lvl{le.lvl}
    { le.logger = nullptr; }
    ~LogEntry();
};

template <class T>
LogEntry operator << (LogEntry && le, T && t) {
    if (le.lvl >= cfg().logLevel())
        cfg().stream() << std::forward<T>(t);
    return std::move(le);
}

}
