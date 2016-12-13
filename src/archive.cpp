#include "archive.hpp"
#include "webmusic.hpp"

Archive::Archive(std::string filename) : _file(filename) {
}

void Archive::add(WebMusic const & wm) {
    _musics.insert({wm.id(), wm.title()});
}

WebMusic Archive::operator [] (std::string const & id) const {
    return WebMusic{id, _musics.at(id)};
}

