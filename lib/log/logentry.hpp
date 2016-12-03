#pragma once

#include "config.hpp"

namespace log {

class Logger;


class LogEntry {
    std::unique_lock<std::mutex> _lock;
public:
    int lvl;

    LogEntry(int lvl);
    LogEntry(LogEntry && le);
    ~LogEntry();
};


template <class T>
LogEntry operator << (LogEntry && le, T && t) {
    if (le.lvl >= cfg().logLevel())
        cfg().stream() << std::forward<T>(t);
    return std::move(le);
}

}
