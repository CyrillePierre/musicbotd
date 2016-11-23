#include <algorithm>
#include <iostream>
#include "player.hpp"

void Player::remove(std::string const & id) {
    Playlist::iterator it = std::find_if(_playlist.begin(), _playlist.end(),
        [&id] (Music const & m) { return m.id == id; });
    if (it != _playlist.end()) _playlist.erase(it);
}

void Player::add(std::string const & id, std::string const & name) {
    Lock lock{_mutex};
    _playlist.push_back(Music{id, name});
}

void Player::add(const Music &m) {
    Lock lock{_mutex};
    _playlist.push_back(m);
}

void Player::remove(Playlist::const_iterator it) {
    Lock lock{_mutex};
    _playlist.erase(it);
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

Player::PlayListView Player::list() const {
    PlayListView plv;
    plv.reserve(_playlist.size());
    for (auto it = _playlist.cbegin(); it != _playlist.cend(); ++it)
        plv.push_back(it);
    return plv;
}

std::size_t Player::playlistSize() const {
    Lock lock{_mutex};
    return _playlist.size();
}

void Player::run() {

}


