#include <algorithm>
#include <iostream>
#include <mpv/client.h>
#include <stdexcept>
#include <thread>
#include "player.hpp"

Player::Player() : _pause{false}, _started{false} {
    _mpv = mpv_create();
    if (!_mpv) std::runtime_error("libmpv: create context failed");
    checkError(mpv_initialize(_mpv));
}

Player::~Player() {
    mpv_terminate_destroy(_mpv);
}

void Player::add(std::string const & id, std::string const & name) {
    _mutex.lock();
    _plv.clear();
    _playlist.push_back(WebMusic{id, name});
    _mutex.unlock();
    _cv.notify_one();
}

void Player::add(const WebMusic &m) {
    _mutex.lock();
    _plv.clear();
    _playlist.push_back(m);
    _mutex.unlock();
    _cv.notify_one();
}

void Player::remove(Playlist::const_iterator it) {
    Lock lock{_mutex};
    _plv.clear();
    _playlist.erase(it);
}

util::Optional<WebMusic> Player::remove(std::string const & id) {
    Playlist::iterator it = std::find_if(_playlist.begin(), _playlist.end(),
        [&id] (WebMusic const & m) { return m.id() == id; });
    if (it != _playlist.end()) {
        auto music = util::makeOptional(std::move(*it));

        Lock lock{_mutex};
        _plv.clear();
        _playlist.erase(it);
        return music;
    }
    return util::Optional<WebMusic>{};
}

util::Optional<WebMusic> Player::remove(std::size_t index) {
    if (index < _plv.size()) {
        Playlist::const_iterator it = _plv[index];
        auto music = util::makeOptional(std::move(*it));

        Lock lock{_mutex};
        _plv.clear();
        _playlist.erase(it);
        return music;
    }
    return util::Optional<WebMusic>{};
}

void Player::clear() {
    Lock lock{_mutex};
    _plv.clear();
    _playlist.clear();
}

void Player::start() {
    std::thread{&Player::run, this}.detach();
}

void Player::stop() {
    std::cout << "[Player] stop" << std::endl;
}

void Player::togglePause() {
    _pause = !_pause;
}

Player::Volume Player::incrVolume(Player::Volume v) {
    std::cout << "[Player] increment volume: " << v << std::endl;
}

Player::PlayListView const & Player::list() const {
    return list(_playlist.size());
}

Player::PlayListView const & Player::list(std::size_t nbLines) const {
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
    playNext();
    while (1) {
        mpv_event *evt = mpv_wait_event(_mpv, 10000);
        std::cout << "event: " << mpv_event_name(evt->event_id) << std::endl;
        if (evt->event_id == MPV_EVENT_SHUTDOWN)
            break;
    }
}

void Player::checkError(int status) const {
    if (status < 0) std::runtime_error(mpv_error_string(status));
}

void Player::playNext() {
    WebMusic currentMusic;
    {
        Lock lock{_mutex};
        _cv.wait(lock, [this] { return !_playlist.empty(); });
        currentMusic = _playlist.front();
        _playlist.pop_front();
    }
    std::string url = currentMusic.url();
    char const * params[] = {"loadfile", url.c_str(), nullptr};
    checkError(mpv_command(_mpv, params));

}


