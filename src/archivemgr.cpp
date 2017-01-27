#include "archivemgr.hpp"
#include <boost/filesystem.hpp>

ArchiveMgr::ArchivePtr ArchiveMgr::load(std::string const & fn) {
    if (fn.find('/') != std::string::npos) return ArchivePtr{};
    if (fn.find('~') != std::string::npos) return ArchivePtr{};
    std::lock_guard<std::mutex> lock{_archivesMutex};
    if(!_archives.count(fn))
		_archives[fn] = std::make_shared<Archive>(_wd+"/"+fn, fn);
	return _archives[fn];
}

void ArchiveMgr::unload(ArchivePtr && ptr) {
	if(ptr.use_count() > 2)	return;
    _archivesMutex.lock();
	_archives.erase(ptr->name());
    _archivesMutex.unlock();
	ptr.reset();
}

ArchiveMgr::Playlists ArchiveMgr::list() {
    using namespace boost::filesystem;
    path workdir{_wd};
    Playlists pls;
    for (directory_entry & entry : directory_iterator{workdir}) {
        path const & p = entry.path();
        if (is_regular_file(p)) {
            std::string filename = p.filename().generic_string();
            if (filename[0] != '.') pls.push_back(filename);
        }
    }
    return std::move(pls);
}
