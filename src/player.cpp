#include <algorithm>
#include <iostream>
#include "player.hpp"

void Player::add(std::string const & id, std::string const & name) {
    Lock lock{_mutex};
    _playlist.push_back(WebMusic{id, name});
}

void Player::add(const WebMusic &m) {
    Lock lock{_mutex};
    _playlist.push_back(m);
}

void Player::remove(Playlist::const_iterator it) {
    Lock lock{_mutex};
    _playlist.erase(it);
}

void Player::remove(std::string const & id) {
    Playlist::iterator it = std::find_if(_playlist.begin(), _playlist.end(),
        [&id] (WebMusic const & m) { return m.id() == id; });
    if (it != _playlist.end()) _playlist.erase(it);
}

void Player::remove(std::size_t index) {
    if (index < _plv.size()) {
        Lock lock{_mutex};
        _playlist.erase(_plv[index]);
    }
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
    _plv.clear();
    _plv.reserve(_playlist.size());
    for (auto it = _playlist.cbegin(); it != _playlist.cend(); ++it)
        _plv.push_back(it);
    return _plv;
}

std::size_t Player::playlistSize() const {
    Lock lock{_mutex};
    return _playlist.size();
}

void Player::run() {

}


