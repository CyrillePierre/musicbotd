#pragma once

#include <sstream>
#include <regex>
#include <curl/curl.h>

class YtNameSolver {
    static std::regex const _reTitle;

    CURL * _curl;
    char _errorBuf[CURL_ERROR_SIZE];
    std::ostringstream _oss;

public:
    YtNameSolver();
    ~YtNameSolver();
    std::string getVideoTitle(std::string url);

private:
    static size_t writeData(void   * data,
                            size_t   size,
                            size_t   nmemb,
                            void   * ptr);
};
