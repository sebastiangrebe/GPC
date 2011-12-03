#ifndef ENTRY_PATH_BUILDER_INCLUDED
#define ENTRY_PATH_BUILDER_INCLUDED
#include <string>
#include <list>
#include "../model/entry.h"

class EntryPathBilder {
public:
	virtual std::list<std::string> buildPath(Entry const& entry) const =0;
	virtual std::string buildPathString(Entry const& entry, bool withOtherEntriesPlaceholder = false) const =0;
};

#endif
