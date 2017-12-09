#pragma once

#include <iosfwd>
#include <list>
#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <util/optional.hpp>
#include <util/any.hpp>
#include <log/log.hpp>
#include <mpv/client.h>
#include "webmusic.hpp"
#include "archive.hpp"

enum PlayerEvt {
    added = 0,
    removed,
    currentChanged,
    volumeChanged,
    paused,
    cleared,
	timePosChanged,
};

/**
 * This class allows to execute a player to play music with a playlist of URLs.
 */
struct Player {
	using Playlist = std::list<WebMusic>;
	using Volume = double;
	using PlayListView = std::vector<Playlist::const_iterator>;
	using Lock = std::unique_lock<std::mutex>;
	using EventHandler = std::function<void(PlayerEvt, util::Any)>;
	using Subscriber = std::pair<std::size_t, std::function<void()>>;
	using Subscribers = std::deque<Subscriber>;

public:
	static constexpr std::size_t playlistMinSize = 2; // see subscriptions
	static constexpr std::size_t playlistMaxSize = 100;

private:
    Playlist                _playlist;
    Archive &               _archive;
    bool	                _pause;
    bool                    _started;
    bool					_isPlaying;
    std::mutex				_playMutex;
    std::thread				_mpvEventThread;
    mutable std::mutex      _mutex;
    mutable std::mutex      _mpvMutex;
    mutable std::mutex      _subscribersMutex;
    std::condition_variable _cv;
    mutable PlayListView	_plv;
    mpv_handle *			_mpv;
    ese::log::Logger		_lg;
    EventHandler			_evtFn;
	Subscribers             _subscribers;
	bool                    _xmas;

public:
    Player(Archive & archive);
    ~Player();

		Archive const&archive() const { return _archive; }

    bool add(std::string const & id, std::string const & name);
    bool add(WebMusic const & m);
    util::Optional<WebMusic> addRandom();
    void remove(Playlist::const_iterator it);
    util::Optional<WebMusic> remove(std::string const & id);
    util::Optional<WebMusic> remove(std::size_t index);
    void clear();
    void start();
    void stop();
    void next();
    void togglePause();
    bool isPaused() const { return _pause; }
	void setPause(bool enable);
    bool isStarted() const { return _started; }
    Volume incrVolume(Volume v);
    Volume volume() const;
    PlayListView const & list() const;
    PlayListView const & list(std::size_t nbLines) const;
    std::size_t playlistSize() const;
    void setEventHandler(EventHandler const & eh) { _evtFn = eh; }
    bool hasCurrent() { return _isPlaying; }
    double duration();
    double timePos();
    WebMusic current();
	void move(double seconds);
	bool xmas() const { return _xmas; }
	void xmas(bool enabled) { _xmas = enabled; }

	bool subscribe(Subscriber const&subscriber);
	bool unsubscribe(Subscriber const&subscriber);

	friend std::ostream &operator<<(std::ostream &os, Player const&player);
	friend std::istream &operator>>(std::istream &is, Player &player);
	
private:
    void run();
    void eventAsync(std::uint64_t userdata, void * data);
    void asyncPlayNext();
    void sendEvent(PlayerEvt pe, util::Any && any = util::Any{});

	void processSubscriptions(std::size_t n);

    template <class... Args, class... Prms>
    void checkError(int (*fn)(Args...), Prms &&... args) const;
};

template <class... Args, class... Prms>
void Player::checkError(int (*fn)(Args...), Prms &&... args) const {
    _mpvMutex.lock();
    int status = fn(std::forward<Prms>(args)...);
    _mpvMutex.unlock();
    if (status < 0) throw std::runtime_error(mpv_error_string(status));
}
