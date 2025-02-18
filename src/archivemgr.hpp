#pragma once

#include <list>
#include <map>
#include <vector>
#include <mutex>
#include "archive.hpp"

class ArchiveMgr {
public:
	using ArchivePtr = std::shared_ptr<Archive>;
	using Playlists  = std::vector<std::string>;

private:
	static constexpr std::size_t STASH_MAX_SIZE = 8;

	std::string const                              _wd;
	std::map<std::string, ArchivePtr>              _archives;
	std::list<std::pair<std::string, ArchivePtr>>  _stashedArchives;
	std::mutex                                     _archivesMutex;

public:
	ArchiveMgr(std::string const & wd = "."): _wd{wd} {}

	ArchivePtr load(std::string const & fn);
	void unload(ArchivePtr && ptr);

    /**
     * @brief list all visible playlists created
     * @note if a playlist name start by a '.' this playlist will not be showed
     * @return the list of playlist name
     */
    Playlists list();
};
