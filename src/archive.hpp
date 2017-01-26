#pragma once

#include <map>
#include <deque>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <log/log.hpp>

class WebMusic;

struct Archive {
    using Musics = std::map<std::string, std::string>;
    using MusicsView = std::deque<Musics::const_iterator>;
    using Lock = std::unique_lock<std::mutex>;
    using Cv   = std::condition_variable;

    static constexpr std::chrono::seconds syncTime{10};

private:
    std::string  _filename;
    std::string  _name;
    Musics       _musics;
    bool         _changed;
    bool         _started;
    Cv           _sleepCv;
    std::thread  _thread;

    ese::log::Logger _lg;

    mutable MusicsView _mv;
    mutable std::mutex _mvMutex;
    mutable std::mutex _musicsMutex;

public:
    Archive(std::string const & filename, std::string const & name = "~");
    ~Archive();
    void add(WebMusic const & wm);
    WebMusic operator [] (std::string const &) const;
    std::size_t size() const noexcept { return _musics.size(); }
    WebMusic random() const;
    bool empty() const noexcept { return _musics.empty(); }

	std::string const & name() const { return _name; }

private:
    void flush();
    void load();
    void syncRoutine();
    void fillMusicsView() const;
};
