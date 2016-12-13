#include "archive.hpp"
#include "webmusic.hpp"

Archive::Archive(std::string filename)
    : _file(filename), _changed{false}
{
}

void Archive::add(WebMusic const & wm) {
    _musics.insert({wm.id(), wm.title()});
    _changed = true;
}

WebMusic Archive::operator [] (std::string const & id) const {
    return WebMusic{id, _musics.at(id)};
}

Archive::MusicsView Archive::random() const {

}

void Archive::flush() {

}

void Archive::load() {

}

void Archive::syncRoutine() {

}

