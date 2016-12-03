#include "logentry.hpp"
#include "logger.hpp"

namespace log {

LogEntry::LogEntry(int lvl) : _lock{cfg().mutex()}, lvl{lvl}
{}

LogEntry::LogEntry(LogEntry && le) : lvl{le.lvl} {
    std::swap(_lock, le._lock);
}

LogEntry::~LogEntry() {
    if (_lock && lvl >= cfg().logLevel())
        cfg().stream() << std::endl;
}

}
