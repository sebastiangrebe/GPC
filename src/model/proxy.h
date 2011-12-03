#ifndef GPC_PROXY_INCLUDED
#define GPC_PROXY_INCLUDED
#include "rule.h"
#include "script.h"
#include <sys/stat.h>
#include <assert.h>

struct Proxy {
	enum Exception {
		RULE_NOT_FOUND,
		NO_MOVE_TARGET_FOUND
	};
	std::list<Rule> rules;
	int index;
	short int permissions;
	std::string fileName;
	Script* dataSource;
	std::list<std::list<std::string> > __idPathList;
	std::list<std::list<std::string> > __idPathList_OtherEntriesPlaceHolders;
	Proxy();
	Proxy(Script& dataSource);
	bool isExecutable() const;
	void set_isExecutable(bool value);
	static std::list<Rule> parseRuleString(const char** ruleString);
	void importRuleString(const char* ruleString);
	Rule* getRuleByEntry(Entry const& entry, std::list<Rule>& list, Rule::RuleType ruletype);
	bool sync(bool deleteInvalidRules = true, bool expand = true);
	void sync_connectExisting(Rule* parent = NULL);
	void sync_add_placeholders(Rule* parent = NULL);
	void sync_expand();
	void sync_cleanup(Rule* parent = NULL);
	bool isModified(Rule const* parent = NULL) const;
	bool deleteFile();
	bool generateFile(std::string const& path, int cfg_dir_prefix_length, std::string const& cfg_dir_noprefix);
	std::string getScriptName();
	Rule& moveRule(Rule* rule, int direction);
private:
	Rule* getParentRule(Rule* child, Rule* root = NULL);
	std::list<Rule>& getRuleList(Rule* parentElement);
	std::list<Rule>::iterator getListIterator(Rule const& needle, std::list<Rule>& haystack);
	Rule* getPlaceholderBySourceList(std::list<Entry> const& sourceList, std::list<Rule>& baseList);
	static void adjustIterator(std::list<Rule>::iterator& iter, int adjustment);
};

#endif
