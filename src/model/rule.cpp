#include "rule.h"

Rule::Rule(Entry& source, bool isVisible, EntryPathFollower& pathFollower, std::list<std::list<std::string> > const& pathesToIgnore, std::list<std::string> const& currentPath) 
	: type(Rule::NORMAL), isVisible(isVisible), __idpath(currentPath), outputName(source.name), dataSource(&source)
{
	if (source.type == Entry::SUBMENU) {
		Rule placeholder(Rule::OTHER_ENTRIES_PLACEHOLDER, currentPath, "*", true);
		placeholder.dataSource = pathFollower.getEntryByPath(currentPath);
		placeholder.dataSource_list = pathFollower.getListByPath(currentPath);
		this->subRules.push_front(placeholder);
	}
	for (std::list<Entry>::iterator iter = source.subEntries.begin(); iter != source.subEntries.end(); iter++) {
		std::list<std::string> currentPath_in_loop = currentPath;
		currentPath_in_loop.push_back(iter->name);

		bool currentPath_in_loop_is_blacklisted = false;
		for (std::list<std::list<std::string> >::const_iterator pti_iter = pathesToIgnore.begin(); pti_iter != pathesToIgnore.end(); pti_iter++) {
			if (*pti_iter == currentPath_in_loop) {
				currentPath_in_loop_is_blacklisted = true;
				break;
			}
		}

		if (!currentPath_in_loop_is_blacklisted){
			this->subRules.push_back(Rule(*iter, isVisible, pathFollower, pathesToIgnore, currentPath_in_loop));
		}
	}
}

std::string Rule::toString(EntryPathBilder const& pathBuilder){
	std::string result = isVisible ? "+" : "-";
	if (dataSource)
		result += pathBuilder.buildPathString(*dataSource, this->type == OTHER_ENTRIES_PLACEHOLDER);
	else if (type == OTHER_ENTRIES_PLACEHOLDER)
		result += "*"; 
	else
		result += "???";
	if (type == NORMAL && (dataSource && dataSource->name != outputName))
		result += " as '"+str_replace("'", "''", outputName)+"'";

	if (this->subRules.size() > 0) {
		result += "{";
		for (std::list<Rule>::iterator iter = this->subRules.begin(); iter != this->subRules.end(); iter++) {
			if (iter != this->subRules.begin())
				result += ", ";
			result += iter->toString(pathBuilder);
		}
		result += "}";
	}
	return result;
}

Rule::Rule(RuleType type, std::list<std::string> path, std::string outputName, bool isVisible)
	: type(type), isVisible(isVisible), __idpath(path), outputName(outputName), dataSource(NULL)
{}

Rule::Rule(RuleType type, std::list<std::string> path, bool isVisible)
	: type(type), isVisible(isVisible), __idpath(path), outputName(path.back()), dataSource(NULL)
{}

std::string Rule::getEntryName() const {
	if (this->dataSource)
		return this->dataSource->name;
	else
		return "?";
}

bool Rule::hasRealSubrules() const {
	for (std::list<Rule>::const_iterator iter = this->subRules.begin(); iter != this->subRules.end(); iter++) {
		if (iter->isVisible && iter->dataSource && iter->type != Rule::OTHER_ENTRIES_PLACEHOLDER && (iter->dataSource->type != Entry::SUBMENU || iter->hasRealSubrules())) {
			return true;
		}
	}
	return false;
}


void Rule::print() const {
	if (this->isVisible && this->dataSource && this->type != Rule::OTHER_ENTRIES_PLACEHOLDER && (this->dataSource->type != Entry::SUBMENU || this->hasRealSubrules())){
		std::cout << (this->dataSource->type == Entry::SUBMENU ? "submenu" : "menuentry");
		std::cout << " \""+this->outputName+"\""+this->dataSource->extension+"{\n";
		if (this->subRules.size() == 0) {
			std::cout << this->dataSource->content;
		} else {
			for (std::list<Rule>::const_iterator iter = this->subRules.begin(); iter != this->subRules.end(); iter++) {
				iter->print();
			}
		}
		std::cout << "}\n";
	}
}
