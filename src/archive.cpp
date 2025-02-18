#include <fstream>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <random>
#include <list>
#include "archive.hpp"
#include "webmusic.hpp"

namespace elog = ese::log;

constexpr std::chrono::seconds Archive::syncTime;

Archive::Archive(std::string const & filename, std::string const & name)
    : _filename(filename), _name(name), _changed{false}, _started{true}
{
    _lg.prefix("archive: ");
    _lg(elog::dbg) << "Archive(" << filename << ")";
    srand(time(nullptr));
    load();
    _thread = std::thread(&Archive::syncRoutine, this);
}

Archive::~Archive() {
    _started = false;
    _sleepCv.notify_one();
    _thread.join();
}

void Archive::add(WebMusic const & wm) {
    _musicsMutex.lock();
    auto it = _musics.insert({wm.id(), wm.title()});
    _musicsMutex.unlock();
    if (it.second) {
        _lg << "add to archive: " << wm.id();
        _changed = true;
    }
    else _lg << "already in archive: " << wm.id();
}

bool Archive::remove(std::string const & id) {
	Lock lock0{_musicsMutex, std::defer_lock}, lock1{_mvMutex, std::defer_lock};
	std::lock(lock0, lock1);

	auto rmIt = _musics.find(id);
	if (rmIt == _musics.end()) return false;

	// Remove the music from the random list
	_mv.erase(std::find_if(_mv.begin(), _mv.end(),
			[&id] (auto const & tmpId) { return tmpId.get() == id; }));

	// Remove the music from the archive
	_musics.erase(rmIt);

	_changed = true;
	return true;
}

WebMusic Archive::operator [] (std::string const & id) const {
    Lock lock{_musicsMutex};
    return WebMusic{id, _musics.at(id)};
}

WebMusic Archive::random() const {
	Lock lock0{_musicsMutex, std::defer_lock}, lock1{_mvMutex, std::defer_lock};
	std::lock(lock0, lock1);

    if (_mv.empty()) fillMusicsView();
	std::string const & id = _mv.front();
    _mv.pop_front();
    return WebMusic{id, _musics.at(id)};
}

std::vector<WebMusic> Archive::search(std::string const&s) const {
	Lock lock{_musicsMutex};
	std::vector<WebMusic> l;

	for(auto const&pair: _musics) {
		std::string const&title = std::get<1>(pair);
		if(std::search(	std::begin(title), std::end(title),
					std::begin(s), std::end(s),
					[](char lhs, char rhs) { return std::toupper(lhs) == std::toupper(rhs); })
			!= std::end(title))
			l.push_back({std::get<0>(pair), title});
	}

	return l;
}

void Archive::flush() {
    _lg(elog::trace) << "flush()";
    std::ofstream file{_filename};
    Lock lock{_musicsMutex};
    for (auto const & music : _musics)
        file << music.first << " " << music.second << "\n";
}

void Archive::load() {
    std::ifstream file{_filename};
    std::string id;
    char buf[1024];

    Lock mLock{_musicsMutex, std::defer_lock};
    Lock mvLock{_mvMutex, std::defer_lock};
    std::lock(mLock, mvLock);

    _musics.clear();
    while (file >> id) {
        file.get();
        file.getline(buf, 1024);
        auto it = _musics.insert({id, std::string{buf}});
        if (it.second) _mv.push_back(it.first->first);
    }
    _lg << "loaded: " << _musics.size() << " musics";

		fillMusicsView();
}

void Archive::syncRoutine() {
    std::mutex mutex;
    _lg << "thread started.";

    while (_started) {
        Lock lock{mutex};
        _sleepCv.wait_for(lock, syncTime, [this] { return !_started; });
        if (_changed) {
            _lg(elog::dbg) << "wake: changed == true";
            _changed = false;
            flush();
        }
        else _lg(elog::trace) << "wake: nothing to do";

//        for (auto const & m : _musics)
//            _lg(elog::dbg) << "map : [" << m.first << "] '" << m.second << "'";
    }

    _lg << "thread finished.";
}

void Archive::fillMusicsView() const {
    _lg(elog::trace) << "fillMusicsView()";
    _mv.clear();
    for (auto it = _musics.cbegin(); it != _musics.end(); ++it)
        _mv.push_back(it->first);
    std::shuffle(_mv.begin(), _mv.end(), std::random_device{});
}
