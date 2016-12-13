#pragma once

#include <stdexcept>
#include <memory>
#include <iostream>
#include <fstream>

namespace ese {
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
          _os(std::addressof(*_file)) {}
    LogStream(LogStream<T> const & ls) = delete;
    LogStream(LogStream<T> &&) = default;

    LogStream & operator = (LogStream<T> && ls) {
        std::swap(ls._file, _file);
        _os = ls._os;
        return *this;
    }

    Stream & stream() {
        if(_os) return *_os;
        else throw std::runtime_error("no stream defined");
    }
};

}
}
