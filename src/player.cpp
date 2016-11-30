#include <algorithm>
#include <iostream>
#include "player.hpp"

void Player::add(std::string const & id, std::string const & name) {
    Lock lock{_mutex};
    _plv.clear();
    _playlist.push_back(WebMusic{id, name});
}

void Player::add(const WebMusic &m) {
    Lock lock{_mutex};
    _plv.clear();
    _playlist.push_back(m);
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

}


