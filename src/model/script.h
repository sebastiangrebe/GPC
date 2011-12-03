

#ifndef GPC_SCRIPT_INCLUDED
#define GPC_SCRIPT_INCLUDED
#include <string>
#include <list>
#include <cstdlib>
#include <sys/stat.h>
#include "entry.h"
#include "../interface/entryPathBuilder.h"
#include "../interface/entryPathFollower.h"
#include "../presenter/commonClass.h"

struct Script : public EntryPathBilder, EntryPathFollower, std::list<Entry>, public CommonClass {
	enum Exception {
		ELEMENT_NOT_FOUND
	};
	std::string name, fileName;
	Script(std::string const& name, std::string const& fileName);
	bool isInScriptDir(std::string const& cfg_dir) const;
	Entry* getEntryByPath(std::list<std::string> const& path);
	Entry* getEntryByName(std::string const& name, std::list<Entry>& parentList);
	std::list<Entry>* getListByPath(std::list<std::string> const& path);
	void moveToBasedir(std::string const& cfg_dir); 
	bool moveFile(std::string const& newPath, short int permissions = -1);
	std::list<std::string> buildPath(Entry const& entry, Entry const* parent) const;
	std::list<std::string> buildPath(Entry const& entry) const;
	std::string buildPathString(Entry const& entry, bool withOtherEntriesPlaceholder = false) const;
};

#endif
