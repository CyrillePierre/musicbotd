#include "ytnamesolver.hpp"

std::regex const YtNameSolver::_reTitle{
    "<meta\\s+itemprop=\"name\"\\s+content=\"([^\"]*)\""};

YtNameSolver::YtNameSolver() : _curl(curl_easy_init()) {
    if (!_curl) throw std::runtime_error("Curl init: failed");
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, &YtNameSolver::writeData);
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(_curl, CURLOPT_ERRORBUFFER, _errorBuf);
}

YtNameSolver::~YtNameSolver() {
    curl_easy_cleanup(_curl);
}

std::string YtNameSolver::getVideoTitle(std::string url) {
    std::string title{"<unknown>"};

    curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());
    CURLcode res = curl_easy_perform(_curl);
    if (!res) {
        std::string data{_oss.str()};
        std::smatch m;
        if (std::regex_search(data, m, _reTitle))
            title = m[1];
    }
    _oss.clear();
    return title;
}

size_t YtNameSolver::writeData(void   * data,
                               size_t   size,
                               size_t   nmemb,
                               void   * ptr)
{
    size_t realSize = size * nmemb;
    YtNameSolver & yns = *static_cast<YtNameSolver *>(ptr);
    yns._oss.write(static_cast<char const *>(data), realSize);
    return realSize;
}
