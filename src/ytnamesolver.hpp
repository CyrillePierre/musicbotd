#pragma once

#include <regex>
#include <exception>
#include <curl/curl.h>

class UnknownVideo : public std::exception {
    std::string _s;

public:
    UnknownVideo(std::string const & url) : _s{"Unknown video: " + url} {}
    char const * what() const noexcept { return _s.c_str(); }
};


class YtNameSolver {
    static std::regex const _reTitle;

    CURL * _curl;
    char _errorBuf[CURL_ERROR_SIZE];

public:
    YtNameSolver();
    ~YtNameSolver();
    std::string getVideoTitle(std::string const & url);

private:
    static size_t writeData(void   * data,
                            size_t   size,
                            size_t   nmemb,
                            void   * ptr);
};
