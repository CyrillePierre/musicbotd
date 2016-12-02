#include "logentry.hpp"
#include "logger.hpp"

namespace log {

LogEntry::~LogEntry() {
    if (logger && lvl >= cfg().logLevel())
        cfg().stream() << std::endl;
}

}
