#include <algorithm>
#include <iostream>
#include <mpv/client.h>
#include <stdexcept>
#include "player.hpp"

Player::Player() : _pause{false}, _started{false} {
    _lg.prefix("player: ");

    _mpv = mpv_create();
    if (!_mpv) std::runtime_error("libmpv: create context failed");

    int opt = 1;
    checkError(mpv_set_property(_mpv, "ytdl", MPV_FORMAT_FLAG, &opt));
    opt = 0;
    checkError(mpv_set_property(_mpv, "video", MPV_FORMAT_FLAG, &opt));
    checkError(mpv_initialize(_mpv));

    _lg << "mpv options:";
    _lg << "  - ytdl   = " << mpv_get_property_string(_mpv, "ytdl");
    _lg << "  - video  = " << mpv_get_property_string(_mpv, "video");
    _lg << "  - volume = " << mpv_get_property_string(_mpv, "volume");

}

Player::~Player() {
    _lg << "mpv destroy";
    if (_started) {
        mpv_terminate_destroy(_mpv);	// kill mpv
        _mpvEventThread.join();			// wait mpv event thread
        Lock lock{_playMutex};			// wait playNext thread
    }
    _lg(log::trace) << "~Player() end";
}

void Player::add(std::string const & id, std::string const & name) {
    _lg(log::trace) << "add(" << id << ", " << name << ')';
    _mutex.lock();
    _plv.clear();
    _playlist.push_back(WebMusic{id, name});
    _mutex.unlock();
    _cv.notify_one();
    sendEvent(PlayerEvt::added);
}

void Player::add(const WebMusic &m) {
    _lg(log::trace) << "add(WebMusic{" << m.id() << ", " << m.title() << "})";
    _mutex.lock();
    _plv.clear();
    _playlist.push_back(m);
    _mutex.unlock();
    _cv.notify_one();
    sendEvent(PlayerEvt::added);
}

void Player::remove(Playlist::const_iterator it) {
    _lg(log::trace) << "remove(iterator = " << &it << ')';
    {
        Lock lock{_mutex};
        _plv.clear();
        _playlist.erase(it);
    }
    sendEvent(PlayerEvt::removed);
}

util::Optional<WebMusic> Player::remove(std::string const & id) {
    _lg(log::trace) << "remove(" << id << ')';

    Playlist::iterator it = std::find_if(_playlist.begin(), _playlist.end(),
        [&id] (WebMusic const & m) { return m.id() == id; });
    if (it != _playlist.end()) {
        auto music = util::makeOptional(std::move(*it));

        {
            Lock lock{_mutex};
            _plv.clear();
            _playlist.erase(it);
        }
        sendEvent(PlayerEvt::removed);
        return music;
    }
    return util::Optional<WebMusic>{};
}

util::Optional<WebMusic> Player::remove(std::size_t index) {
    _lg(log::trace) << "remove(" << index << ')';

    if (index < _plv.size()) {
        Playlist::const_iterator it = _plv[index];
        auto music = util::makeOptional(std::move(*it));

        {
            Lock lock{_mutex};
            _plv.clear();
            _playlist.erase(it);
        }
        sendEvent(PlayerEvt::removed);
        return music;
    }
    return util::Optional<WebMusic>{};
}

void Player::clear() {
    _lg(log::trace) << "clear()";
    {
        Lock lock{_mutex};
        _plv.clear();
        _playlist.clear();
    }
    sendEvent(PlayerEvt::cleared);
}

void Player::start() {
    _lg(log::trace) << "start()";
    _started = true;
    _mpvEventThread = std::thread{&Player::run, this};
}

void Player::stop() {
    _lg(log::trace) << "stop()";
    _started = false;
    _cv.notify_all();
}

void Player::togglePause() {
    _lg(log::trace) << "togglePause()";
    _pause = !_pause;
    _lg << "pause state = " << _pause;
    sendEvent(PlayerEvt::paused);
}

Player::Volume Player::incrVolume(Player::Volume v) {
    _lg(log::trace) << "incrVolume(" << v << ')';
    sendEvent(PlayerEvt::volumeChanged);
}

Player::PlayListView const & Player::list() const {
    _lg(log::trace) << "list()";
    return list(_playlist.size());
}

Player::PlayListView const & Player::list(std::size_t nbLines) const {
    _lg(log::trace) << "list(" << nbLines << ')';

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

void Player::run() {
    _lg << "mpv thread created";
    while (_started) {
        mpv_event *evt = mpv_wait_event(_mpv, 10000);
        _lg(log::dbg) << "mpv event: " << mpv_event_name(evt->event_id);
        switch (evt->event_id) {
        case MPV_EVENT_SHUTDOWN: stop();          break;
        case MPV_EVENT_IDLE:     asyncPlayNext(); break;
        }
    }
    _lg << "mpv thread finished";
}

void Player::checkError(int status) const {
    if (status < 0) std::runtime_error(mpv_error_string(status));
}

void Player::asyncPlayNext() {
    _lg(log::trace) << "asyncPlayNext()";
    if (!_playMutex.try_lock()) return;

    Lock lock{_playMutex, std::adopt_lock};
    std::thread([this, playLock = std::move(lock)] {
        WebMusic currentMusic;
        {
            Lock lock{_mutex};
            _cv.wait(lock, [this] { return !_playlist.empty() || !_started; });
            if (!_started) return;
            currentMusic = _playlist.front();
            _playlist.pop_front();
        }
        sendEvent(PlayerEvt::currentChanged);
        std::string url = currentMusic.url();
        char const * params[] = {"loadfile", url.c_str(), nullptr};
        checkError(mpv_command(_mpv, params));
    }).detach();
}

void Player::sendEvent(PlayerEvt pe) {
    _lg << "sending event: " << (int)pe;
    if (_evtFn) _evtFn(pe);
}
