#pragma once

#include <fstream>
#include <map>

class WebMusic;

class Archive {
    std::fstream                       _file;
    std::map<std::string, std::string> _musics;

public:
    Archive(std::string filename);
    void add(WebMusic const & wm);
    WebMusic operator [] (std::string const &) const;
    std::size_t size() const noexcept { return _musics.size(); }
    WebMusic random() const;
};
