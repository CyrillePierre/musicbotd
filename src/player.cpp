#include <algorithm>
#include <iostream>
#include <mpv/client.h>
#include <stdexcept>
#include "player.hpp"

namespace elog = ese::log;

Player::Player(Archive & archive)
    : _archive{archive}, _pause{false}, _started{false}, _isPlaying{false}
{
    _lg.prefix("player: ");

    _mpv = mpv_create();
    if (!_mpv) std::runtime_error("libmpv: create context failed");

	_lg(elog::dbg) << "initializing mpv";
    checkError(&mpv_initialize, _mpv);

	int status;
    int opt = 1;

	_lg(elog::dbg) << "Set ytdl property";
    if ((status = mpv_set_property(_mpv, "ytdl", MPV_FORMAT_FLAG, &opt)) < 0)
        _lg(elog::err) << "failed: " << mpv_error_string(status);

    opt = 0;

	_lg(elog::dbg) << "Set video property";
	if ((status = mpv_set_property(_mpv, "video", MPV_FORMAT_FLAG, &opt)) < 0)
		_lg(elog::err) << "failed: " << mpv_error_string(status);

	_lg(elog::dbg) << "Request info property";
    checkError(&mpv_request_log_messages, _mpv, "info");

    _lg << "mpv options:";
    _lg << "  - version = " << mpv_get_property_string(_mpv, "mpv-version");
    _lg << "  - ytdl    = " << mpv_get_property_string(_mpv, "ytdl");
    _lg << "  - video   = " << mpv_get_property_string(_mpv, "video");

}

Player::~Player() {
    _lg << "mpv destroy";
    if (_started) {
        mpv_terminate_destroy(_mpv);	// kill mpv
        _mpvEventThread.join();			// wait mpv event thread
//        Lock lock{_playMutex};			// wait playNext thread
    }
    _lg(elog::trace) << "~Player() end";
}

bool Player::add(std::string const & id, std::string const & name) {
    _lg(elog::trace) << "add(" << id << ", " << name << ')';
    return add(WebMusic{id, name});
}

bool Player::add(const WebMusic &m) {
    _lg(elog::trace) << "add(WebMusic{" << m.id() << ", " << m.title() << "})";
//    if (_playlist.size() > playlistMaxSize) {
//        _lg(elog::warn) << "Playlist is full.";
//        return false;
//    }
    _archive.add(m);
    _mutex.lock();
    _plv.clear();
    _playlist.push_back(m);
    _mutex.unlock();
    sendEvent(PlayerEvt::added, _playlist.back());
    _cv.notify_one();
    return true;
}

util::Optional<WebMusic> Player::addRandom() {
    _lg(elog::trace) << "addRandom()";
    if (!_archive.empty()/* && _playlist.size() < playlistMaxSize*/) {
        util::Optional<WebMusic> wm{_archive.random()};
        _mutex.lock();
        _playlist.push_back(wm);
        _mutex.unlock();
        sendEvent(PlayerEvt::added, _playlist.back());
        _cv.notify_one();
        return wm;
    }
    return util::Optional<WebMusic>{};
}

void Player::remove(Playlist::const_iterator it) {
    _lg(elog::trace) << "remove(iterator = " << &it << ')';
    WebMusic wm = std::move(*it);
    {
        Lock lock{_mutex};
        _plv.clear();
        _playlist.erase(it);
    }
    sendEvent(PlayerEvt::removed, std::move(wm));
}

util::Optional<WebMusic> Player::remove(std::string const & id) {
    _lg(elog::trace) << "remove(" << id << ')';

    Playlist::iterator it = std::find_if(_playlist.begin(), _playlist.end(),
        [&id] (WebMusic const & m) { return m.id() == id; });
    if (it != _playlist.end()) {
        auto music = util::makeOptional(std::move(*it));

        {
            Lock lock{_mutex};
            _plv.clear();
            _playlist.erase(it);
        }
        sendEvent(PlayerEvt::removed, music.get());
        return music;
    }
    return util::Optional<WebMusic>{};
}

util::Optional<WebMusic> Player::remove(std::size_t index) {
    _lg(elog::trace) << "remove(" << index << ')';

    _mutex.lock();
    std::size_t size = _plv.size();
    _mutex.unlock();

    if (index < size) {
        _mutex.lock();
        Playlist::const_iterator it = _plv[index];
        auto music = util::makeOptional(std::move(*it));
        _plv.clear();
        _playlist.erase(it);
        _mutex.unlock();
        sendEvent(PlayerEvt::removed, music.get());
        return music;
    }
    return util::Optional<WebMusic>{};
}

void Player::clear() {
    _lg(elog::trace) << "clear()";
    {
        Lock lock{_mutex};
        _plv.clear();
        _playlist.clear();
    }
    sendEvent(PlayerEvt::cleared);
}

void Player::start() {
    _lg(elog::trace) << "start()";
    _started = true;
    _mpvEventThread = std::thread{&Player::run, this};
}

void Player::stop() {
    _lg(elog::trace) << "stop()";
    _started = false;
    _cv.notify_all();
}

void Player::next() {
    _lg(elog::trace) << "next()";
    char const * params[] = {"stop", nullptr};
    checkError(&mpv_command, _mpv, params);
}

void Player::togglePause() {
    _lg(elog::trace) << "togglePause()";
    int pauseState;
    checkError(&mpv_get_property, _mpv, "pause", MPV_FORMAT_FLAG, &pauseState);
    _lg(elog::dbg) << "current pause state = " << pauseState;
    pauseState = !pauseState;
    checkError(&mpv_set_property_async, _mpv, PlayerEvt::paused, "pause", MPV_FORMAT_FLAG, &pauseState);
    _lg << "pause state = " << pauseState;
    _pause = pauseState;
		sendEvent(PlayerEvt::paused, _pause);
}

Player::Volume Player::incrVolume(Player::Volume v) {
    _lg(elog::trace) << "incrVolume(" << v << ')';
    Player::Volume vol = volume();
    vol += v;
		if(vol < 0)   vol = 0;
		if(vol > 150) vol = 150;
    checkError(&mpv_set_property_async, _mpv, PlayerEvt::volumeChanged, "volume", MPV_FORMAT_DOUBLE, &vol);
		sendEvent(PlayerEvt::volumeChanged, vol);
    return vol;
}

Player::Volume Player::volume() {
    _lg(elog::trace) << "volume()";
    Player::Volume vol;
    checkError(&mpv_get_property, _mpv, "volume", MPV_FORMAT_DOUBLE, &vol);
    _lg(elog::dbg) << "current volume = " << vol;
    return vol;
}

Player::PlayListView const & Player::list() const {
    _lg(elog::trace) << "list()";
    return list(_playlist.size());
}

Player::PlayListView const & Player::list(std::size_t nbLines) const {
    _lg(elog::trace) << "list(" << nbLines << ')';

    Lock lock{_mutex};
    _plv.clear();
    _plv.reserve(_playlist.size());

    std::size_t i = 0;
    auto it = _playlist.cbegin();
    for (; i < nbLines && it != _playlist.cend(); ++it, ++i)
        _plv.push_back(it);
    return _plv;
}

std::size_t Player::playlistSize() const {
    Lock lock{_mutex};
    return _playlist.size();
}

double Player::duration() {
    _lg(elog::trace) << "duration()";
    if (!_isPlaying) return 0./0.;
    double val;
    checkError(&mpv_get_property, _mpv, "duration", MPV_FORMAT_DOUBLE, &val);
    _lg(elog::dbg) << "duration = " << val << " s";
    return val;
}

double Player::timePos() {
    _lg(elog::trace) << "timePos()";
    if (!_isPlaying) return 0./0.;
    double time;
    checkError(&mpv_get_property, _mpv, "time-pos", MPV_FORMAT_DOUBLE, &time);
    _lg(elog::dbg) << "time = " << time << " s";
    return time;
}

WebMusic Player::current() {
    _lg(elog::trace) << "current()";
    std::string id, title;
    _mpvMutex.lock();
    char * tmp = mpv_get_property_string(_mpv, "path");
    if (tmp) id = tmp;
    mpv_free(tmp);
    tmp = mpv_get_property_string(_mpv, "media-title");
    if (tmp) title = tmp;
    mpv_free(tmp);
    _mpvMutex.unlock();
    _lg(elog::dbg) << "url = " << id;
    _lg(elog::dbg) << "media-title = " << title;
    return WebMusic{id.substr(id.size() - cfg::ytIdSize), title};
}

void Player::run() {
    std::string text;

    _lg << "mpv thread created";
    while (_started) {
        mpv_event *evt = mpv_wait_event(_mpv, 10000);
        if (evt->event_id != MPV_EVENT_LOG_MESSAGE)
            _lg(elog::dbg) << "mpv event: " << mpv_event_name(evt->event_id);

        switch (evt->event_id) {
        case MPV_EVENT_SHUTDOWN:    stop();             break;
        case MPV_EVENT_IDLE:        asyncPlayNext();    break;
        case MPV_EVENT_END_FILE:	_isPlaying = false; break;
        case MPV_EVENT_START_FILE:	_isPlaying = true;  break;
        case MPV_EVENT_LOG_MESSAGE:
            text = static_cast<mpv_event_log_message*>(evt->data)->text;
            text.pop_back();
            _lg(elog::dbg) << "mpv log: " << text;
            break;
        default: break;
        }
    }
    _lg << "mpv thread finished";
}

void Player::asyncPlayNext() {
    _lg(elog::trace) << "asyncPlayNext()";
    if (!_playMutex.try_lock()) return;

    Lock lock{_playMutex, std::adopt_lock};
    std::thread([this, playLock = std::move(lock)] {
        WebMusic currentMusic;
        _isPlaying = false;
        {
            Lock lock{_mutex};
            _cv.wait(lock, [this] { return !_playlist.empty() || !_started; });
            _lg(elog::dbg) << "playlist thread waked up";
            if (!_started) return;
            currentMusic = _playlist.front();
            _playlist.pop_front();
        }
        sendEvent(PlayerEvt::currentChanged, currentMusic);
        std::string url = currentMusic.url();
        char const * params[] = {"loadfile", url.c_str(), nullptr};
        checkError(&mpv_command, _mpv, params);
    }).detach();
}

void Player::sendEvent(PlayerEvt pe, util::Any && any) {
    _lg << "sending event: " << (int)pe;
    if (_evtFn) _evtFn(pe, std::move(any));
}
