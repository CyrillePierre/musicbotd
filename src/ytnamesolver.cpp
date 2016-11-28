#include <sstream>
#include "ytnamesolver.hpp"

std::regex const YtNameSolver::_reTitle{
    "<meta\\s+itemprop=\"name\"\\s+content=\"([^\"]*)\""};


YtNameSolver::YtNameSolver() : _curl(curl_easy_init()) {
    if (!_curl) throw std::runtime_error("Curl init: failed");
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, &YtNameSolver::writeData);
    curl_easy_setopt(_curl, CURLOPT_ERRORBUFFER, _errorBuf);
}


YtNameSolver::~YtNameSolver() {
    curl_easy_cleanup(_curl);
}


std::string YtNameSolver::getVideoTitle(std::string const & url) {
    std::ostringstream oss;
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &oss);
    curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());
    CURLcode res = curl_easy_perform(_curl);

    if (!res) {
        std::string data{oss.str()};
        oss.clear();

        std::smatch m;
        if (std::regex_search(data, m, _reTitle))
            return m[1];
    }
    else oss.clear();
    throw UnknownVideo{url};
}


size_t YtNameSolver::writeData(void   * data,
                               size_t   size,
                               size_t   nmemb,
                               void   * ptr)
{
    size_t realSize = size * nmemb;
    std::ostringstream & oss = *static_cast<std::ostringstream *>(ptr);
    oss.write(static_cast<char const *>(data), realSize);
    return realSize;
}
