#pragma once

namespace log {

enum LogLevel {
    trace,
    dbg,
    msg,
    warn,
    err,
    crit
};

constexpr std::array<char const *, 6> levels {
    "trace",
    "debug",
    "msg",
    "warning",
    "error",
    "critical"
};

}
