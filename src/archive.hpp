#pragma once

#include <fstream>
#include <map>
#include <vector>
#include <mutex>

class WebMusic;

struct Archive {
    using Musics = std::map<std::string, std::string>;
    using MusicsView = std::vector<Musics::const_iterator>;

    static constexpr std::chrono::seconds syncTime{60};

private:
    std::fstream _file;
    Musics       _musics;
    std::mutex   _mutex;
    bool         _changed;

public:
    Archive(std::string filename);
    void add(WebMusic const & wm);
    WebMusic operator [] (std::string const &) const;
    std::size_t size() const noexcept { return _musics.size(); }
    WebMusic random() const;

private:
    void flush();
    void load();
    void syncRoutine();
};
