#ifndef ENTRY_PATH_FOLLOWER_INCLUDED
#define ENTRY_PATH_FOLLOWER_INCLUDED
#include <string>
#include <list>
#include "../model/entry.h"

class EntryPathFollower {
public:
	virtual Entry* getEntryByPath(std::list<std::string> const& path)=0;
	virtual std::list<Entry>* getListByPath(std::list<std::string> const& path)=0;
};

#endif
