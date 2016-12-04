#pragma once

#include <list>
#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <util/optional.hpp>
#include <log/log.hpp>
#include "webmusic.hpp"
#include "mpv/client.h"

enum class PlayerEvt {
    added,
    removed,
    currentChanged,
    volumeChanged,
    paused,
    cleared,
};

/**
 * This class allows to execute a player to play music with a playlist of URLs.
 */
struct Player {
    using Playlist = std::list<WebMusic>;
    using Volume = unsigned short;
    using PlayListView = std::vector<Playlist::const_iterator>;
    using Lock = std::unique_lock<std::mutex>;
    using EventHandler = std::function<void(PlayerEvt)>;

private:
    Playlist                _playlist;
    bool	                _pause;
    bool                    _started;
    std::mutex				_playMutex;
    std::thread				_mpvEventThread;
    mutable std::mutex      _mutex;
    std::condition_variable _cv;
    mutable PlayListView	_plv;
    mpv_handle *			_mpv;
    log::Logger				_lg;
    EventHandler			_evtFn;

public:
    Player();
    ~Player();

    void add(std::string const & id, std::string const & name);
    void add(WebMusic const & m);
    void remove(Playlist::const_iterator it);
    util::Optional<WebMusic> remove(std::string const & id);
    util::Optional<WebMusic> remove(std::size_t index);
    void clear();
    void start();
    void stop();
    void togglePause();
    bool isPaused() const { return _pause; }
    bool isStarted() const { return _started; }
    Volume incrVolume(Volume v);
    PlayListView const & list() const;
    PlayListView const & list(std::size_t nbLines) const;
    std::size_t playlistSize() const;
    void setEventHandler(EventHandler const & eh) { _evtFn = eh; }

private:
    void run();
    void checkError(int status) const;
    void asyncPlayNext();
    void sendEvent(PlayerEvt pe);
};
