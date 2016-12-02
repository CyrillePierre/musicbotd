#pragma once

#include <memory>
#include <iostream>
#include <fstream>
#include <array>
#include <ctime>

namespace util {
namespace log {

template <class T>
class LogStream {
    using Fstream = std::basic_fstream<T>;
public:
    using Stream = std::basic_ostream<T>;
private:
    std::unique_ptr<Fstream> _file;
    Stream * _os;
public:
    LogStream() : _os{nullptr} {}
    LogStream(Stream & os) : _os{&os} {}
    LogStream(std::string const & filename)
        : _file{std::make_unique<Fstream>(filename, std::ios::app)},
          _os{&_file.get()} {}
    LogStream(LogStream<T> const & ls) = delete;
    LogStream(LogStream<T> &&) = default;

    LogStream & operator = (LogStream<T> && ls) {
        std::swap(ls._file, _file);
        _os = ls._os;
    }

    Stream & stream() {
        if(_os) return *_os;
        else throw std::runtime_error("no stream defined");
    }
};

enum LogLevel {
    trace,
    dbg,
    msg,
    warn,
    err,
    crit
};

constexpr std::array<char const *, 5> levels {
    "trace"
    "debug",
    "msg",
    "warning",
    "error",
    "critical"
};

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


class Logger;

struct LogEntry {
    Logger * logger;
    int      lvl;

    LogEntry(Logger * l, int lvl) : logger{l}, lvl{lvl} {}
    LogEntry(LogEntry && le) : logger{le.logger}, lvl{le.lvl}
    { le.logger = nullptr; }
    ~LogEntry();
};

class Logger {
    std::string _prefix;

public:
    void prefix(std::string const & prefix) { _prefix = prefix; }

    LogEntry operator () (int lvl) {
        Config & c = cfg();
        int cfgLvl = c.logLevel();

        if (lvl >= cfgLvl) {
            if (c.timeEnabled()) addTime();
            if (lvl < levels.size())
                c.stream() << "[" << levels[lvl] << "] ";
            c.stream() << _prefix;
        }
        return LogEntry{this, lvl};
    }

private:
    void addTime() {
        std::time_t t = time(nullptr);
        char buf[32];
        if (std::strftime(buf, 32, "%F %T", std::localtime(&t)))
            cfg().stream() << buf << ' ';
    }
};

inline LogEntry::~LogEntry() {
    if (logger && lvl >= cfg().logLevel())
        cfg().stream() << std::endl;
}

template <class T>
LogEntry operator << (Logger & l, T && t) {
    return l(msg) << std::forward<T>(t);
}

template <class T>
LogEntry operator << (LogEntry && le, T && t) {
    if (le.lvl >= cfg().logLevel())
        cfg().stream() << std::forward<T>(t);
    return std::move(le);
}

}
}
