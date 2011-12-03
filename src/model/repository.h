#ifndef GPC_REPOSITORY_INCLUDED
#define GGPC_REPOSITORY_INCLUDED
#include "script.h"
#include "libproxyscript_parser.h"
#include "pscriptname_translator.h"
#include <sys/stat.h>
#include <dirent.h>
#include "../presenter/commonClass.h"

struct Repository : public std::list<Script>, public CommonClass {
	void load(std::string const& directory, bool is_proxifiedScript_dir);
	Script* getScriptByFilename(std::string const& fileName, bool createScriptIfNotFound = false);
	Script* getNthScript(int pos);
	void deleteAllEntries();
};

#endif
