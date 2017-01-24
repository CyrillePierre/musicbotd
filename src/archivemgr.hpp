#pragma once

#include <map>
#include "archive.hpp"

class ArchiveMgr {
public:
	using ArchivePtr = std::shared_ptr<Archive>;

private:
	std::string const _wd;
	std::map<std::string, ArchivePtr> _archives;

public:
	ArchiveMgr(std::string const & wd = "."): _wd{wd} {}

	ArchivePtr load(std::string const & fn);
	void unload(ArchivePtr && ptr);
};
