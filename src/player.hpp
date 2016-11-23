#pragma once

#include <list>
#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>

struct Music {
    std::string id;
    std::string name;

    bool operator == (Music const & m) const { return id == m.id; }
};

/**
 * This class allows to execute a player to play music with a playlist of URLs.
 */
struct Player {
    using Playlist = std::list<Music>;
    using Volume = unsigned short;
    using PlayListView = std::vector<Playlist::const_iterator>;
    using Lock = std::unique_lock<std::mutex>;

private:
    Playlist                _playlist;
    bool	                _pause;
    bool                    _started;
    mutable std::mutex      _mutex;
    std::condition_variable _cv;

public:
    Player() : _pause{false}, _started{false} {}

    void add(std::string const & id, std::string const & name);
    void add(Music const & m);
    void remove(Playlist::const_iterator it);
    void remove(std::string const & id);
    void start();
    void stop();
    void togglePause();
    bool isPaused() const { return _pause; }
    Volume incrVolume(Volume v);
    PlayListView list() const;
    std::size_t playlistSize() const;

private:
    void run();
};
