#include <ctime>
#include "logger.hpp"

namespace ese {
namespace log {

LogEntry Logger::operator () (int lvl) const {
    Config & c = cfg();

    LogEntry le{lvl};	// locking stream mutex
    if (lvl >= c.logLevel()) {
        if (c.timeEnabled()) addTime();
        if ((std::size_t)lvl < levels.size())
            c.stream() << "[" << levels[lvl] << "] ";
        c.stream() << _prefix;
    }
    return std::move(le);
}


void Logger::addTime() const {
    std::time_t t = time(nullptr);
    char buf[32];
    if (std::strftime(buf, 32, cfg().timeFormat(), std::localtime(&t)))
        cfg().stream() << buf << ' ';
}

}
}
