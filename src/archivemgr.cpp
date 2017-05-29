#include "archivemgr.hpp"
#include <boost/filesystem.hpp>

ArchiveMgr::ArchivePtr ArchiveMgr::load(std::string const & fn) {
	if (fn.find('/') != std::string::npos) return ArchivePtr{};
	if (fn.find('~') != std::string::npos) return ArchivePtr{};

	std::lock_guard<std::mutex> lock{_archivesMutex};

	auto it = std::find_if(std::begin(_stashedArchives), std::end(_stashedArchives),
		[&fn](std::pair<std::string, ArchivePtr> const&pair) {
			return std::get<0>(pair) == fn;
		}
	);

	if(it != std::end(_stashedArchives)) {
		_archives[std::get<0>(*it)] = std::get<1>(*it);
		_stashedArchives.erase(it);
	}

	if(!_archives.count(fn))
		_archives[fn] = std::make_shared<Archive>(_wd+"/"+fn, fn);
	return _archives[fn];
}

void ArchiveMgr::unload(ArchivePtr && ptr) {
	if(ptr.use_count() > 2)	return;
	_archivesMutex.lock();
	_archives.erase(ptr->name());
	_archivesMutex.unlock();

	_stashedArchives.push_front(std::make_pair(ptr->name(), ptr));
	if(_stashedArchives.size() > STASH_MAX_SIZE)
		_stashedArchives.pop_back();
}

ArchiveMgr::Playlists ArchiveMgr::list() {
    using namespace boost::filesystem;
    path workdir{_wd};
    Playlists pls;
    for (directory_iterator it{workdir}; it != directory_iterator{}; ++it) {
        path const & p = it->path();
        if (is_regular_file(p)) {
            std::string filename = p.filename().generic_string();
            if (filename[0] != '.') pls.push_back(filename);
        }
    }
    return std::move(pls);
}
