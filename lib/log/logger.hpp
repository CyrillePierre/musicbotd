#pragma once

#include "logentry.hpp"
#include "loglevel.hpp"

namespace log {

class Logger {
    std::string _prefix;

public:
    void prefix(std::string const & prefix) { _prefix = prefix; }
    LogEntry operator () (int lvl) const;

private:
    void addTime() const;
};

template <class T>
LogEntry operator << (Logger const & l, T && t) {
    return l(msg) << std::forward<T>(t);
}

}
