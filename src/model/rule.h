#ifndef GPC_RULE_INCLUDED
#define GPC_RULE_INCLUDED
#include <string>
#include "entry.h"
#include "../interface/entryPathBuilder.h"
#include "../interface/entryPathFollower.h"
#include <iostream>

struct Rule {
	Entry* dataSource; 
	std::list<Entry>* dataSource_list; 
	std::string outputName;
	std::list<std::string> __idpath; 
	bool isVisible;
	std::list<Rule> subRules;
	enum RuleType {
		NORMAL, OTHER_ENTRIES_PLACEHOLDER
	} type;
	Rule(RuleType type, std::list<std::string> path, std::string outputName, bool isVisible);
	Rule(RuleType type, std::list<std::string> path, bool isVisible);
	Rule(Entry& source, bool isVisible, EntryPathFollower& pathFollower, std::list<std::list<std::string> > const& pathesToIgnore = std::list<std::list<std::string> >(), std::list<std::string> const& currentPath = std::list<std::string>()); 
	Rule();
	std::string toString(EntryPathBilder const& pathBuilder);
	bool hasRealSubrules() const;
	void print() const;
	std::string getEntryName() const;
};

#endif
