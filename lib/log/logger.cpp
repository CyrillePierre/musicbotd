#include <ctime>
#include "logger.hpp"

namespace log {

LogEntry Logger::operator ()(int lvl) {
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


void Logger::addTime() {
    std::time_t t = time(nullptr);
    char buf[32];
    if (std::strftime(buf, 32, "%F %T", std::localtime(&t)))
        cfg().stream() << buf << ' ';
}

}
