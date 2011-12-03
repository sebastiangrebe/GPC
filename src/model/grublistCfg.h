#ifndef GPC_GrublistCfg_INCLUDED
#define GPC_GrublistCfg_INCLUDED
#include <list>
#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include <cstdio>
#include <sstream>
#include <map>
#include <libintl.h>
#include <unistd.h>

#include "proxylist.h"
#include "repository.h"
#include "libproxyscript_parser.h"
#include "../config.h"
#include "mountTable.h"
#include "settings_mng_ds.h"
#include "grubEnv.h"
#include "../interface/evt_model.h"

#include "../interface/mutex.h"

#include "../presenter/commonClass.h"


class GrublistCfg : public CommonClass {
	EventListener_model* eventListener;
	
	double progress;
	Mutex* mutex;
public:
	GrublistCfg(GrubEnv& env);
	void setEventListener(EventListener_model& eventListener);
	void setMutex(Mutex& mutex);
	void setLogger(Logger& logger);
	
	enum Exception {
		GRUB_CFG_DIR_NOT_FOUND,
		GRUB_CMD_EXEC_FAILED,
		NO_MOVE_TARGET_FOUND,
		MISSING_MUTEX
	};

	ProxyList proxies;
	Repository repository;
	
	bool verbose;
	bool error_proxy_not_found;
	GrubEnv& env;
	void lock();
	bool lock_if_free();
	void unlock();
	
	bool cancelThreadsRequested;
	bool createScriptForwarder(std::string const& scriptName) const;
	bool removeScriptForwarder(std::string const& scriptName) const;
	std::string readScriptForwarder(std::string const& scriptForwarderFilePath) const;
	void load(bool preserveConfig = false);
	void save();
	void readGeneratedFile(FILE* source, bool createScriptIfNotFound = false);
	bool loadStaticCfg();

	void send_new_load_progress(double newProgress);
	void send_new_save_progress(double newProgress);
	void cancelThreads();
	void reset();
	double getProgress() const;
	void increaseProxyPos(Proxy* proxy);
	void renumerate();
	
	void swapRules(Rule* a, Rule* b);
	Rule& moveRule(Rule* rule, int direction);
	void swapProxies(Proxy* a, Proxy* b);
	
	bool cfgDirIsClean();
	void cleanupCfgDir();
	
	bool compare(GrublistCfg const& other) const;
	static std::list<Rule const*> getComparableRules(std::list<Rule> const& list);
	static bool compareLists(std::list<Rule const*> a, std::list<Rule const*> b);
	
	void renameRule(Rule* rule, std::string const& newName);
};

#endif
