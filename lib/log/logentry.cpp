#include "logentry.hpp"
#include "logger.hpp"

namespace log {

LogEntry::~LogEntry() {
    if (_token && lvl >= cfg().logLevel())
        cfg().stream() << std::endl;
}

}
