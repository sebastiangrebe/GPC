#include "grublistCfg.h"

GrublistCfg::GrublistCfg(GrubEnv& env)
 : error_proxy_not_found(false),
 progress(0),
 cancelThreadsRequested(false), verbose(true), env(env), eventListener(NULL),
 mutex(NULL)
{}

void GrublistCfg::setEventListener(EventListener_model& eventListener) {
	this->eventListener = &eventListener;
}

void GrublistCfg::setMutex(Mutex& mutex) {
	this->mutex = &mutex;
}

void GrublistCfg::setLogger(Logger& logger) {
	this->CommonClass::setLogger(logger);
	this->proxies.setLogger(logger);
	this->repository.setLogger(logger);
}

void GrublistCfg::lock(){
	if (this->mutex == NULL)
		throw MISSING_MUTEX;
	this->mutex->lock();
}
bool GrublistCfg::lock_if_free(){
	if (this->mutex == NULL)
		throw MISSING_MUTEX;
	return this->mutex->trylock();
}
void GrublistCfg::unlock(){
	if (this->mutex == NULL)
		throw MISSING_MUTEX;
	this->mutex->unlock();
}

bool GrublistCfg::createScriptForwarder(std::string const& scriptName) const {
	std::string scriptNameNoPath = scriptName.substr((this->env.cfg_dir+"/proxifiedScripts/").length());
	std::string outputFilePath = this->env.cfg_dir+"/LS_"+scriptNameNoPath;
	FILE* existingScript = fopen(outputFilePath.c_str(), "r");
	if (existingScript == NULL){
		FILE* fwdScript = fopen(outputFilePath.c_str(), "w");
		if (fwdScript){
			fputs("#!/bin/sh\n", fwdScript);
			fputs(("'"+scriptName.substr(env.cfg_dir_prefix.length())+"'").c_str(), fwdScript);
			fclose(fwdScript);
			chmod(outputFilePath.c_str(), 0755);
			return true;
		}
		else
			return false;
	}
	else {
		fclose(existingScript);
		return false;
	}
}

bool GrublistCfg::removeScriptForwarder(std::string const& scriptName) const {
	std::string scriptNameNoPath = scriptName.substr((this->env.cfg_dir+"/proxifiedScripts/").length());
	std::string filePath = this->env.cfg_dir+"/LS_"+scriptNameNoPath;
	return unlink(filePath.c_str()) == 0;
}

std::string GrublistCfg::readScriptForwarder(std::string const& scriptForwarderFilePath) const {
	std::string result;
	FILE* scriptForwarderFile = fopen(scriptForwarderFilePath.c_str(), "r");
	if (scriptForwarderFile){
		int c;
		while ((c = fgetc(scriptForwarderFile)) != EOF && c != '\n'){} 
		if (c != EOF)
			while ((c = fgetc(scriptForwarderFile)) != EOF && c != '\n'){result += char(c);} 
		fclose(scriptForwarderFile);
	}
	return result.substr(1, result.length()-2);
}

void GrublistCfg::load(bool preserveConfig){
	if (!preserveConfig){
		send_new_load_progress(0);

		DIR* hGrubCfgDir = opendir(this->env.cfg_dir.c_str());

		if (!hGrubCfgDir){
			throw GRUB_CFG_DIR_NOT_FOUND;
		}

		this->log("loading scripts…", Logger::EVENT);
		this->lock();
		repository.load(this->env.cfg_dir, false);
		repository.load(this->env.cfg_dir+"/proxifiedScripts", true);
		this->unlock();
		send_new_load_progress(0.05);
	
		this->log("loading proxies…", Logger::EVENT);
		this->lock();
		struct dirent *entry;
		struct stat fileProperties;
		while (entry = readdir(hGrubCfgDir)){
			stat((this->env.cfg_dir+"/"+entry->d_name).c_str(), &fileProperties);
			if ((fileProperties.st_mode & S_IFMT) != S_IFDIR){ 
				if (entry->d_name[2] == '_' && entry->d_name[0] != '0'){
					this->proxies.push_back(Proxy());
					this->proxies.back().fileName = this->env.cfg_dir+"/"+entry->d_name;
					this->proxies.back().index = (entry->d_name[0]-'0')*10 + (entry->d_name[1]-'0');
					this->proxies.back().permissions = fileProperties.st_mode & ~S_IFMT;
				
					FILE* proxyFile = fopen((this->env.cfg_dir+"/"+entry->d_name).c_str(), "r");
					ProxyScriptData data(proxyFile);
					fclose(proxyFile);
					if (data){
						this->proxies.back().dataSource = repository.getScriptByFilename(this->env.cfg_dir_prefix+data.scriptCmd);
						this->proxies.back().importRuleString(data.ruleString.c_str());
					}
					else {
						this->proxies.back().dataSource = repository.getScriptByFilename(this->env.cfg_dir+"/"+entry->d_name);
						this->proxies.back().importRuleString("+*");
					}
				
				}
			}
		}
		closedir(hGrubCfgDir);
		this->proxies.sort();
		this->unlock();
	}
	else {
		this->lock();
		repository.deleteAllEntries();
		this->unlock();
	}
	
	this->log("creating proxifiedScript links & chmodding other files…", Logger::EVENT);

	this->lock();
	for (Repository::iterator iter = this->repository.begin(); iter != this->repository.end(); iter++){
		if (iter->isInScriptDir(env.cfg_dir)){
			createScriptForwarder(iter->fileName);
			std::list<Proxy*> relatedProxies = proxies.getProxiesByScript(*iter);
			for (std::list<Proxy*>::iterator piter = relatedProxies.begin(); piter != relatedProxies.end(); piter++){
				int res = chmod((*piter)->fileName.c_str(), 0644);
			}
		} else {
			chmod(iter->fileName.c_str(), 0755);
		}
	}
	this->unlock();
	send_new_load_progress(0.1);
	
	this->log("running " + this->env.mkconfig_cmd, Logger::EVENT);
	FILE* mkconfigProc = popen(this->env.mkconfig_cmd.c_str(), "r");
	readGeneratedFile(mkconfigProc);
	
	int success = pclose(mkconfigProc);
	if (success != 0 && !cancelThreadsRequested){
		throw GRUB_CMD_EXEC_FAILED;
	}
	this->log("mkconfig successfull completed", Logger::INFO);

	this->send_new_load_progress(0.9);

	mkconfigProc = NULL;
	
	this->log("restoring grub configuration", Logger::EVENT);
	this->lock();
	for (Repository::iterator iter = this->repository.begin(); iter != this->repository.end(); iter++){
		if (iter->isInScriptDir(env.cfg_dir)){
			bool result = removeScriptForwarder(iter->fileName);
			if (!result) {
				this->log("removing of script forwarder not successful!", Logger::ERROR);
			}
		}
		std::list<Proxy*> relatedProxies = proxies.getProxiesByScript(*iter);
		for (std::list<Proxy*>::iterator piter = relatedProxies.begin(); piter != relatedProxies.end(); piter++){
			chmod((*piter)->fileName.c_str(), (*piter)->permissions);
		}
	}
	this->unlock();
	
	this->log("loading completed", Logger::EVENT);
	send_new_load_progress(1);
}


void GrublistCfg::readGeneratedFile(FILE* source, bool createScriptIfNotFound){
	GrubConfRow row;
	Script* script;
	int i = 0;
	while (!cancelThreadsRequested && (row = GrubConfRow(source))){
		if (row.text.substr(0,10) == ("### BEGIN ") && row.text.substr(row.text.length()-4,4) == " ###"){
			this->lock();
			if (script)
				this->proxies.sync_all(true, true, script);
			std::string scriptName = row.text.substr(10, row.text.length()-14);
			std::string prefix = this->env.cfg_dir_prefix;
			std::string realScriptName = prefix+scriptName;
			if (realScriptName.substr(0, (this->env.cfg_dir+"/LS_").length()) == this->env.cfg_dir+"/LS_"){
				realScriptName = prefix+readScriptForwarder(realScriptName);
			}
			script = repository.getScriptByFilename(realScriptName, createScriptIfNotFound);
			if (createScriptIfNotFound){ 
				this->proxies.push_back(Proxy(*script));
			}
			this->unlock();
			if (script){
				this->send_new_load_progress(0.1 + (0.7 / this->repository.size() * ++i));
			}
		}
		else if (script != NULL && row.text.substr(0, 10) == "menuentry ") {
			this->lock();
			Entry newEntry(source, row, this->getLoggerPtr());
			script->push_back(newEntry);
			this->proxies.sync_all(false, false, script);
			this->unlock();
			this->send_new_load_progress(0.1 + (0.7 / this->repository.size() * i));
		} else if (script != NULL && row.text.substr(0, 8) == "submenu ") {
			this->lock();
			Entry newEntry(source, row, this->getLoggerPtr());
			script->push_back(newEntry);
			this->proxies.sync_all(false, false, script);
			this->unlock();
			this->send_new_load_progress(0.1 + (0.7 / this->repository.size() * i));
		}
	}
	this->lock();
	if (script)
		this->proxies.sync_all(true, true, script);
	this->unlock();
}

void GrublistCfg::save(){
	send_new_save_progress(0);
	std::map<std::string, int> samename_counter;
	proxies.deleteAllProxyscriptFiles(); 
	proxies.clearTrash(); 
	for (Repository::iterator script_iter = repository.begin(); script_iter != repository.end(); script_iter++)
		script_iter->moveToBasedir(this->env.cfg_dir);
	
	send_new_save_progress(0.1);

	int mkdir_result = mkdir((this->env.cfg_dir+"/proxifiedScripts").c_str(), 0755); 

	int proxyCount = 0;
	for (Repository::iterator script_iter = repository.begin(); script_iter != repository.end(); script_iter++){
		std::list<Proxy*> relatedProxies = proxies.getProxiesByScript(*script_iter);
		if (proxies.proxyRequired(*script_iter)){
			script_iter->moveFile(this->env.cfg_dir+"/proxifiedScripts/"+pscriptname_encode(script_iter->name, samename_counter[script_iter->name]++), 0755);
			for (std::list<Proxy*>::iterator proxy_iter = relatedProxies.begin(); proxy_iter != relatedProxies.end(); proxy_iter++){
				std::ostringstream nameStream;
				nameStream << (*proxy_iter)->index << "_" << script_iter->name << "_proxy";
				(*proxy_iter)->generateFile(this->env.cfg_dir+"/"+nameStream.str(), this->env.cfg_dir_prefix.length(), this->env.cfg_dir_noprefix);
				proxyCount++;
			}
		}
		else {
			if (relatedProxies.size() == 1){
				std::ostringstream nameStream;
				nameStream << relatedProxies.front()->index << "_" << script_iter->name;
				script_iter->moveFile(this->env.cfg_dir+"/"+nameStream.str(), relatedProxies.front()->permissions);
			}
			else {
				this->log("GrublistCfg::save: cannot move proxy… only one expected!", Logger::ERROR);
			}
		}	
	}
	send_new_save_progress(0.2);
	
	{
		int proxifiedScriptCount = 0;
		struct dirent *entry;
		struct stat fileProperties;
		DIR* hScriptDir = opendir((this->env.cfg_dir+"/proxifiedScripts").c_str());
		while (entry = readdir(hScriptDir)){
			if (std::string(entry->d_name) != "." && std::string(entry->d_name) != ".."){
				proxifiedScriptCount++;
			}
		}
		closedir(hScriptDir);
		
		if (proxifiedScriptCount == 0)
			rmdir((this->env.cfg_dir+"/proxifiedScripts").c_str());
	}
	
	FILE* proxyBin = fopen((this->env.cfg_dir+"/bin/grubcfg_proxy").c_str(), "r");
	bool proxybin_exists = proxyBin != NULL;
	bool proxy_is_dummy = false;
	std::string dummyproxy_code = "#!/bin/sh\ncat\n";
	std::string proxy_code;
	
	if (proxyBin){
		this->log("proxybin does already exist!", Logger::ERROR);
		int c;
		for (int i = 0; i < dummyproxy_code.length() && (c = fgetc(proxyBin)) != EOF; i++)
			proxy_code += c;
		
		if (proxy_code == dummyproxy_code)
			proxy_is_dummy = true;
		fclose(proxyBin);
	}

	if (proxyCount != 0 && (!proxybin_exists || proxy_is_dummy)){

		int bin_mk_success = mkdir((this->env.cfg_dir+"/bin").c_str(), 0755);

		FILE* proxyBinSource = fopen((std::string(LIBDIR)+"/grubcfg-proxy").c_str(), "r");
		
		if (proxyBinSource){
			FILE* proxyBinTarget = fopen((this->env.cfg_dir+"/bin/grubcfg_proxy").c_str(), "w");
			if (proxyBinTarget){
				int c;
				while ((c = fgetc(proxyBinSource)) != EOF){
					fputc(c, proxyBinTarget);
				}
				fclose(proxyBinTarget);
				chmod((this->env.cfg_dir+"/bin/grubcfg_proxy").c_str(), 0755);
			} else {
				this->log("could not open proxy output file!", Logger::ERROR);
			}
			fclose(proxyBinSource);
		} else {
			this->log("proxy could not be copied, generating dummy!", Logger::ERROR);
			FILE* proxyBinTarget = fopen((this->env.cfg_dir+"/bin/grubcfg_proxy").c_str(), "w");
			if (proxyBinTarget){
				fputs(dummyproxy_code.c_str(), proxyBinTarget);
				error_proxy_not_found = true;
				fclose(proxyBinTarget);
				chmod((this->env.cfg_dir+"/bin/grubcfg_proxy").c_str(), 0755);
			} else {
				this->log("coundn't create proxy!", Logger::ERROR);
			}
		}
	}
	else if (proxyCount == 0 && proxybin_exists){
		unlink((this->env.cfg_dir+"/bin/grubcfg_proxy").c_str());
		rmdir((this->env.cfg_dir+"/bin").c_str());
	}


	FILE* saveProc = popen((env.update_cmd+" 2>&1").c_str(), "r");
	if (saveProc) {
		int c;
		std::string row = "";
		while ((c = fgetc(saveProc)) != EOF) {
			if (c == '\n') {
				send_new_save_progress(0.5); 
				this->log(row, Logger::INFO);
				row = "";
			} else {
				row += char(c);
			}
		}
		pclose(saveProc);
	}
	send_new_save_progress(1);
}

bool GrublistCfg::loadStaticCfg(){
	FILE* oldConfigFile = fopen(env.output_config_file.c_str(), "r");
	if (oldConfigFile){
		this->readGeneratedFile(oldConfigFile, true);
		fclose(oldConfigFile);
		return true;
	}
	return false;
}

void GrublistCfg::renameRule(Rule* rule, std::string const& newName){
	rule->outputName = newName;
}

bool GrublistCfg::compare(GrublistCfg const& other) const {
	std::list<const Rule*> rlist[2];
	for (int i = 0; i < 2; i++){
		const GrublistCfg* gc = i == 0 ? this : &other;
		for (ProxyList::const_iterator piter = gc->proxies.begin(); piter != gc->proxies.end(); piter++){
			if (piter->isExecutable() && piter->dataSource){
				std::string fname = piter->dataSource->fileName.substr(other.env.cfg_dir.length()+1);
				if (i == 0 || fname[0] >= '1' && fname[0] <= '9' && fname[1] >= '0' && fname[1] <= '9' && fname[2] == '_'){
					std::list<Rule const*> comparableRules = this->getComparableRules(piter->rules);
					rlist[i].splice(rlist[i].end(), comparableRules);
				}
			}
		}
	}
	return GrublistCfg::compareLists(rlist[0], rlist[1]);
}

std::list<Rule const*> GrublistCfg::getComparableRules(std::list<Rule> const& list) {
	std::list<Rule const*> result;
	for (std::list<Rule>::const_iterator riter = list.begin(); riter != list.end(); riter++){
		if (riter->type == Rule::NORMAL && riter->dataSource && riter->isVisible){
			result.push_back(&*riter);
		}
	}
	return result;
}

bool GrublistCfg::compareLists(std::list<Rule const*> a, std::list<Rule const*> b) {
	if (a.size() != b.size())
		return false;

	std::list<const Rule*>::iterator self_iter = a.begin(), other_iter = b.begin();
	while (self_iter != a.end() && other_iter != b.end()){
		if ((*self_iter)->outputName != (*other_iter)->outputName || (*self_iter)->dataSource->extension != (*other_iter)->dataSource->extension || (*self_iter)->dataSource->content != (*other_iter)->dataSource->content || (*self_iter)->dataSource->type != (*other_iter)->dataSource->type)
			return false;
		assert((*self_iter)->dataSource && (*other_iter)->dataSource);
		assert((*self_iter)->dataSource->type == (*other_iter)->dataSource->type);
		if ((*self_iter)->dataSource->type == Entry::SUBMENU && !GrublistCfg::compareLists(GrublistCfg::getComparableRules((*self_iter)->subRules), GrublistCfg::getComparableRules((*other_iter)->subRules))) {
			return false;
		}
		self_iter++;
		other_iter++;
	}
	return true;
}


void GrublistCfg::send_new_load_progress(double newProgress){
	if (this->eventListener != NULL){
		this->progress = newProgress;
		this->eventListener->loadProgressChanged();
	}
	else if (this->verbose) {
		this->log("cannot show updated load progress - no UI connected!", Logger::ERROR);
	}
}

void GrublistCfg::send_new_save_progress(double newProgress){
	if (this->eventListener != NULL){
		this->progress = newProgress;
		this->eventListener->saveProgressChanged();
	}
	else if (this->verbose) {
		this->log("cannot show updated save progress - no UI connected!", Logger::ERROR);
	}
}

void GrublistCfg::cancelThreads(){
	cancelThreadsRequested = true;
}


void GrublistCfg::reset(){
	this->lock();
	this->repository.clear();
	this->proxies.clear();
	this->unlock();
}

double GrublistCfg::getProgress() const {
	return progress;
}

void GrublistCfg::renumerate(){
	short int i = 10;
	for (ProxyList::iterator iter = this->proxies.begin(); iter != this->proxies.end(); iter++){
		iter->index = i++;
	}
	this->proxies.sort();
}

Rule& GrublistCfg::moveRule(Rule* rule, int direction){
	try {
		return this->proxies.getProxyByRule(rule)->moveRule(rule, direction);
	} catch (Proxy::Exception e) {
		if (e == Proxy::NO_MOVE_TARGET_FOUND) {
			throw GrublistCfg::NO_MOVE_TARGET_FOUND;
		} else {
			throw e;
		}
	}
}

void GrublistCfg::swapProxies(Proxy* a, Proxy* b){
	int index1 = a->index;
	a->index = b->index;
	b->index = index1;
	this->proxies.sort();
}

bool GrublistCfg::cfgDirIsClean(){
	DIR* hGrubCfgDir = opendir(this->env.cfg_dir.c_str());
	if (hGrubCfgDir){
		struct dirent *entry;
		struct stat fileProperties;
		while (entry = readdir(hGrubCfgDir)){
			std::string fname = entry->d_name;
			if (fname.length() >= 4 && fname.substr(0,3) == "LS_" || fname.substr(0,3) == "PS_" || fname.substr(0,3) == "DS_")
				return false;
		}
		closedir(hGrubCfgDir);
	}
	return true;
}
void GrublistCfg::cleanupCfgDir(){
	this->log("cleaning up cfg dir!", Logger::IMPORTANT_EVENT);
	
	DIR* hGrubCfgDir = opendir(this->env.cfg_dir.c_str());
	if (hGrubCfgDir){
		struct dirent *entry;
		struct stat fileProperties;
		std::list<std::string> lsfiles, dsfiles, psfiles;
		std::list<std::string> proxyscripts;
		while (entry = readdir(hGrubCfgDir)){
			std::string fname = entry->d_name;
			if (fname.length() >= 4){
				if (fname.substr(0,3) == "LS_")
					lsfiles.push_back(fname);
				else if (fname.substr(0,3) == "DS_")
					dsfiles.push_back(fname);
				else if (fname.substr(0,3) == "PS_")
					psfiles.push_back(fname);

				else if (fname[0] >= '1' && fname[0] <= '9' && fname[1] >= '0' && fname[1] <= '9' && fname[2] == '_')
					proxyscripts.push_back(fname);
			}
		}
		closedir(hGrubCfgDir);
		
		for (std::list<std::string>::iterator iter = lsfiles.begin(); iter != lsfiles.end(); iter++){
			this->log("deleting " + *iter, Logger::EVENT);
			unlink((this->env.cfg_dir+"/"+(*iter)).c_str());
		}
		
		for (std::list<std::string>::iterator iter = proxyscripts.begin(); iter != proxyscripts.end(); iter++){
			this->log("re-activating " + *iter, Logger::EVENT);
			chmod((this->env.cfg_dir+"/"+(*iter)).c_str(), 0755);
		}

		
		for (std::list<std::string>::iterator iter = dsfiles.begin(); iter != dsfiles.end(); iter++) {
			this->log("renaming " + *iter, Logger::EVENT);
			rename((this->env.cfg_dir+"/"+(*iter)).c_str(), (this->env.cfg_dir+"/"+iter->substr(3)).c_str());
		}

		int i = 20; 
		for (std::list<std::string>::iterator iter = psfiles.begin(); iter != psfiles.end(); iter++) {
			this->log("renaming " + *iter, Logger::EVENT);
			std::string out = *iter;
			out.replace(0, 2, (std::string("") + char('0' + (i/10)%10) + char('0' + i%10)));
			rename((this->env.cfg_dir+"/"+(*iter)).c_str(), (this->env.cfg_dir+"/"+out).c_str());
			i++;
		}
	}
}





