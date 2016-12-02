#pragma once

#include <stdexcept>
#include <memory>
#include <iostream>
#include <fstream>

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

}
