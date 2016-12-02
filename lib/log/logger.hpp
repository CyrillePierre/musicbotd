#pragma once

#include "logentry.hpp"
#include "loglevel.hpp"

namespace log {

class Logger {
    std::string _prefix;

public:
    void prefix(std::string const & prefix) { _prefix = prefix; }
    LogEntry operator () (int lvl);

private:
    void addTime();
};

template <class T>
LogEntry operator << (Logger & l, T && t) {
    return l(msg) << std::forward<T>(t);
}

}
