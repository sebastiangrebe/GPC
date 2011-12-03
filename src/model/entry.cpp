#include "entry.h"

GrubConfRow::GrubConfRow(FILE* sourceFile)
	: eof(false), is_loaded(true)
{
	this->eof = true;
	int c;
	while ((c = fgetc(sourceFile)) != EOF){
		this->eof = false;
		if (c != '\n'){
			this->text += char(c);
		}
		else {
			break;
		}
	}
}
GrubConfRow::GrubConfRow()
	: eof(false), is_loaded(false)
{}
GrubConfRow::operator bool(){
	return !eof && is_loaded;
}

std::string str_replace(const std::string &search, const std::string &replace, std::string subject) {
	size_t pos = 0;
	while (pos < subject.length() && (pos = subject.find(search, pos)) != -1){
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}

Entry::Entry()
	: isValid(false)
{}

Entry::Entry(std::string name, std::string extension, std::string content, EntryType type)
	: name(name), extension(extension), content(content), isValid(true), type(type)
{}
Entry::Entry(FILE* sourceFile, GrubConfRow firstRow, Logger* logger)
	: isValid(false), type(MENUENTRY)
{

	GrubConfRow row;
	bool inEntry = false;
	while ((row = firstRow) || (row = GrubConfRow(sourceFile))){
		if (inEntry && (row.text.substr(0, 10) == "menuentry " || row.text.substr(0, 8) == "submenu ")){
			this->subEntries.push_back(Entry(sourceFile, row));
		} else if (inEntry && row.text != "}") {
			this->content += row.text+"\n";
		} else if (inEntry && row.text == "}") {
			isValid = true;
			inEntry = false;
			break; 
		} else if (!inEntry && row.text.substr(0, 10) == "menuentry "){
			int endOfEntryName = row.text.find('"', 12);
			if (endOfEntryName == -1)
				endOfEntryName = row.text.find('\'', 12);
			std::string entryName = row.text.substr(11, endOfEntryName-11);

			std::string extension = row.text.substr(endOfEntryName+1, row.text.length()-(endOfEntryName+1)-1);

			*this = Entry(entryName, extension);
			inEntry = true;
		} else if (!inEntry && row.text.substr(0, 8) == "submenu ") {
			int endOfEntryName = row.text.find('"', 10);
			if (endOfEntryName == -1)
				endOfEntryName = row.text.find('\'', 10);
			std::string entryName = row.text.substr(9, endOfEntryName-9);

			*this = Entry(entryName, "", "", SUBMENU);
			if (logger) {
				this->setLogger(*logger);
			}
			inEntry = true;
		} else {
			this->log("unmatched row: " + row.text, Logger::ERROR);
		}
		firstRow.eof = true; 
	}
}

std::list<Entry>& Entry::getSubEntries() {
	return this->subEntries;
}

Entry::operator bool() const {
	return isValid;
}


