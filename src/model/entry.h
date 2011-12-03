#ifndef GPC_ENTRY_INCLUDED
#define GPC_ENTRY_INCLUDED
#include <cstdio>
#include <string>
#include <list>
#include "../presenter/commonClass.h"

struct GrubConfRow {
	GrubConfRow(FILE* sourceFile);
	GrubConfRow();
	std::string text;
	bool eof;
	bool is_loaded;
	operator bool();
};

std::string str_replace(const std::string &search, const std::string &replace, std::string subject);

struct Entry : public CommonClass {
	enum EntryType {
		MENUENTRY,
		SUBMENU,
		ROOT_ENTRY
	} type;
	bool isValid;
	std::string name, extension, content;
	std::list<Entry> subEntries;
	Entry();
	Entry(std::string name, std::string extension, std::string content = "", EntryType type = MENUENTRY);
	Entry(FILE* sourceFile, GrubConfRow firstRow = GrubConfRow(), Logger* logger = NULL);
	std::list<Entry>& getSubEntries();
	operator bool() const;
};

#endif
