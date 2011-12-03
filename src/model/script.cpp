#include "script.h"

Script::Script(std::string const& name, std::string const& fileName)
	: name(name), fileName(fileName)
{}
bool Script::isInScriptDir(std::string const& cfg_dir) const {
	return this->fileName.substr(cfg_dir.length(), std::string("/proxifiedScripts/").length()) == "/proxifiedScripts/";
}
Entry* Script::getEntryByPath(std::list<std::string> const& path){
	Entry* result = NULL;
	for (std::list<std::string>::const_iterator iter = path.begin(); iter != path.end(); iter++) {
		result = this->getEntryByName(*iter, result != NULL ? result->subEntries : *this);
		if (result == NULL)
			return NULL;
	}
	return result;
}

Entry* Script::getEntryByName(std::string const& name, std::list<Entry>& parentList) {
	for (Script::iterator iter = parentList.begin(); iter != parentList.end(); iter++){
		if (iter->name == name)
			return &*iter;
	}
	return NULL;
}

std::list<Entry>* Script::getListByPath(std::list<std::string> const& path) {
	if (path.size()) {
		Entry* e = this->getEntryByPath(path);
		if (e == NULL) {
			return NULL;
		} else {
			return &e->subEntries;
		}
	} else {
		return this;
	}
}

void Script::moveToBasedir(std::string const& cfg_dir){
	std::string newPath;
	if (isInScriptDir(cfg_dir)){
		newPath = cfg_dir+"/PS_"+this->fileName.substr((cfg_dir+"/proxifiedScripts/").length());
	}
	else {
		newPath = cfg_dir+"/DS_"+this->fileName.substr(cfg_dir.length()+1);
	}
	int renameSuccess = rename(this->fileName.c_str(), newPath.c_str());
	if (renameSuccess == 0)
		this->fileName = newPath;
}

bool Script::moveFile(std::string const& newPath, short int permissions){
	int rename_success = rename(this->fileName.c_str(), newPath.c_str());
	if (rename_success == 0){
		this->fileName = newPath;
		if (permissions != -1)
			chmod(this->fileName.c_str(), permissions);
		return true;
	}
	return false;
}

std::list<std::string> Script::buildPath(Entry const& entry, Entry const* parent) const {
	std::list<Entry> const& list = parent ? parent->subEntries : *this;
	for (std::list<Entry>::const_iterator iter = list.begin(); iter != list.end(); iter++) {
		if (&*iter == &entry) {
			std::list<std::string> result;
			result.push_back(iter->name);
			return result;
		}
		if (iter->type == Entry::SUBMENU) {
			try {
				std::list<std::string> result = this->buildPath(entry, &*iter);
				result.push_front(iter->name);
				return result;
			} catch (Script::Exception e) {
				
			}
		}
	}
	throw ELEMENT_NOT_FOUND;
}


std::list<std::string> Script::buildPath(Entry const& entry) const {
	return this->buildPath(entry, NULL);
}

std::string Script::buildPathString(Entry const& entry, bool withOtherEntriesPlaceholder) const {
	std::string result;
	std::list<std::string> list = this->buildPath(entry, NULL);
	for (std::list<std::string>::iterator iter = list.begin(); iter != list.end(); iter++) {
		if (result != "") {
			result += "/";
		}
		result += "'"+str_replace("'", "''", *iter)+"'";
	}

	if (withOtherEntriesPlaceholder) {
		result += "/*";
	}
	return result;
}


