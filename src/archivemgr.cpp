#include "archivemgr.hpp"

ArchiveMgr::ArchivePtr ArchiveMgr::load(std::string const & fn) {
	if(!_archives.count(fn))
		_archives[fn] = std::make_shared<Archive>(_wd+"/"+fn, fn);
	return _archives[fn];
}

void ArchiveMgr::unload(ArchivePtr && ptr) {
	if(ptr.use_count() > 2)	return;
	_archives.erase(ptr->name());
	ptr.reset();
}
