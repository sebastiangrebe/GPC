#ifndef GPC_PROXYLIST_INCLUDED
#define GPC_PROXYLIST_INCLUDED
#include <list>
#include <sstream>
#include "proxy.h"
#include "../presenter/commonClass.h"

struct EntryTitleListItem {
	std::string labelPathValue;
	std::string labelPathLabel;
	std::string numericPathValue;
	std::string numericPathLabel;
};
struct ProxyList : public std::list<Proxy>, public CommonClass {
	enum Exception {
		NO_RELATED_PROXY_FOUND
	};
	std::list<Proxy> trash;
	std::list<Proxy*> getProxiesByScript(Script const& script);
	std::list<const Proxy*> getProxiesByScript(Script const& script) const;
	void sync_all(bool deleteInvalidRules = true, bool expand = true, Script* relatedScript = NULL); 
	bool proxyRequired(Script const& script) const;
	void deleteAllProxyscriptFiles();
	static bool compare_proxies(Proxy const& a, Proxy const& b);
	void sort();
	void deleteProxy(Proxy* proxyPointer);
	void clearTrash();
	std::list<EntryTitleListItem> generateEntryTitleList() const;
	static std::list<EntryTitleListItem> generateEntryTitleList(std::list<Rule> const& parent, std::string const& labelPathPrefix, std::string const& numericPathPrefix, std::string const& numericPathLabelPrefix);
	Proxy* getProxyByRule(Rule* rule, std::list<Rule> const& list, Proxy& parentProxy);
	Proxy* getProxyByRule(Rule* rule);
};

#endif
