#include "settings_mng_ds.h"

SettingRow::SettingRow() : isActive(true), hasExportPrefix(false), isSetting(true) {}

void SettingRow::validate(){
	isActive = false;
	hasExportPrefix = false;
	isSetting = false;

	if (name != "" && value != "" && (name.length() < 2 || name.substr(0,2) != "# ")){
		isSetting = true;
		if (name[0] != '#')
			isActive = true;
		else
			name = name.substr(1);
		
		if (name.length() > 7 && name.substr(0,7) == "export "){
			hasExportPrefix = true;
			name = name.substr(7);
		}
	}
	else {
		name = "";
		value = "";
	}
}

std::string SettingRow::getOutput(){
	if (isSetting) {
		if (name != "") {
			return (isActive ? "" : "#")+std::string(hasExportPrefix ? "export " : "")+name+"=\""+value+"\""+(comment != "" ? " #"+comment : "");
		} else {
			return "#UNNAMED_OPTION=\""+value+"\""; 
		}
	} else {
		return plaintext;
	}
}

SettingsManagerDataStore::SettingsManagerDataStore(GrubEnv& env)
	: _reloadRequired(false), env(env)
{
}

bool SettingsManagerDataStore::reloadRequired() const {
	return this->_reloadRequired;
}

std::list<SettingRow>::iterator SettingsManagerDataStore::begin(bool jumpOverPlaintext){
	std::list<SettingRow>::iterator iter = settings.begin();
	if (jumpOverPlaintext)
		iter_to_next_setting(iter);
	return iter;
}
std::list<SettingRow>::iterator SettingsManagerDataStore::end(){
	return settings.end();
}
void SettingsManagerDataStore::iter_to_next_setting(std::list<SettingRow>::iterator& iter){
	iter++;
	while (iter != settings.end()){
		if (iter->isSetting)
			break;
		else
			iter++;
	}
}
std::string SettingsManagerDataStore::getValue(std::string const& name){
	for (std::list<SettingRow>::iterator iter = this->begin(); iter != this->end(); this->iter_to_next_setting(iter)){
		if (name == iter->name)
			return iter->value;
	}
	return "";
}
bool SettingsManagerDataStore::setValue(std::string const& name, std::string const& value){
	for (std::list<SettingRow>::iterator iter = this->begin(); iter != this->end(); this->iter_to_next_setting(iter)){
		if (name == iter->name){
			
			if (iter->value != value){ 
				iter->value = value;
				if (name == "GRUB_DISABLE_LINUX_RECOVERY" || name == "GRUB_DISABLE_OS_PROBER")
					_reloadRequired = true;
			}
			
			return true;
		}
	}

	settings.push_back(SettingRow());
	settings.back().name = name;
	settings.back().value = value;
	settings.back().validate();
	if (name == "GRUB_DISABLE_LINUX_RECOVERY" || name == "GRUB_DISABLE_OS_PROBER")
		_reloadRequired = true;
	return false;
}

std::string SettingsManagerDataStore::addNewItem(){
	SettingRow newRow;
	newRow.name = "";
	settings.push_back(newRow);
	return newRow.name;
}

void SettingsManagerDataStore::removeItem(std::string const& name){
	for (std::list<SettingRow>::iterator iter = this->begin(); iter != this->end(); this->iter_to_next_setting(iter)){
		if (iter->name == name){
			settings.erase(iter);
			break; 
		}
	}
}

void SettingsManagerDataStore::renameItem(std::string const& old_name, std::string const& new_name){
	for (std::list<SettingRow>::iterator iter = this->begin(); iter != this->end(); this->iter_to_next_setting(iter)){
		if (iter->name == old_name){
			iter->name = new_name;
			break; 
		}
	}
}

bool SettingsManagerDataStore::isActive(std::string const& name, bool checkValueToo){
	for (std::list<SettingRow>::iterator iter = this->begin(); iter != this->end(); this->iter_to_next_setting(iter)){
		if (name == iter->name)
			return iter->isActive && (!checkValueToo || iter->value != "false");
	}
}
bool SettingsManagerDataStore::setIsActive(std::string const& name, bool value){
	for (std::list<SettingRow>::iterator iter = this->begin(); iter != this->end(); this->iter_to_next_setting(iter)){
		if (name == iter->name){
			if (iter->isActive != value){
				iter->isActive = value;
				if (name == "GRUB_DISABLE_LINUX_RECOVERY" || name == "GRUB_DISABLE_OS_PROBER")
					_reloadRequired = true;
			}
			return true;
		}
	}
	return false;
}

bool SettingsManagerDataStore::setIsExport(std::string const& name, bool isExport){
	for (std::list<SettingRow>::iterator iter = this->begin(); iter != this->end(); this->iter_to_next_setting(iter)){
		if (name == iter->name){
			if (iter->hasExportPrefix != isExport){
				iter->hasExportPrefix = isExport;
			}
			return true;
		}
	}
	return false;
}

bool SettingsManagerDataStore::load(){
	settings.clear();

	FILE* file = fopen(this->env.settings_file.c_str(), "r");
	if (file){
		std::string row;
		int c;
		int step = 0; 
		bool inQuotes = false;
		char quoteChar;
		settings.push_back(SettingRow());
		while ((c = fgetc(file)) != EOF){
			if (c == '\n'){
				settings.back().validate();
				settings.push_back(SettingRow());
				inQuotes = false;
				step = 0;
			}
			else {
				settings.back().plaintext += c;
				if (c == '"' && (!inQuotes || quoteChar == c)){
					inQuotes = !inQuotes;
					quoteChar = '"';
				}
				else if (c == '\'' && (!inQuotes || quoteChar == c)){
					inQuotes = !inQuotes;
					quoteChar = '\'';
				}
				else {
					if (step == 0 && c == '=' && !inQuotes)
						step = 1;
					else if (step == 0)
						settings.back().name += c;
					else if (step == 1)
						settings.back().value += c;
				}
			}
		}
		if (settings.size() > 0)
			settings.back().validate();
		
		fclose(file);
		return true;
	}
	else
		return false;
}

bool SettingsManagerDataStore::save(){
	const char* background_script = "\
#! /bin/sh -e\n\
# Name of this script: 'grub_background.sh'\n\
\n\
   WALLPAPER=\"/usr/share/images/desktop-base/background\"\n\
   COLOR_NORMAL=\"light-gray/black\"\n\
   COLOR_HIGHLIGHT=\"magenta/black\"\n\
\n\
if [ \"${GRUB_MENU_PICTURE}\" ] ; then\n\
   echo \"using custom appearance settings\" >&2\n\
   [ \"${GRUB_COLOR_NORMAL}\" ] && COLOR_NORMAL=\"${GRUB_COLOR_NORMAL}\"\n\
   [ \"${GRUB_COLOR_HIGHLIGHT}\" ] && COLOR_HIGHLIGHT=\"${GRUB_COLOR_HIGHLIGHT}\"\n\
   WALLPAPER=\"${GRUB_MENU_PICTURE}\"\n\
fi\n";

	FILE* outFile = fopen(this->env.settings_file.c_str(), "w");
	if (outFile){
		bool background_script_required = false;
		for (std::list<SettingRow>::iterator iter = this->begin(false); iter != this->end(); iter++){
			if (iter != this->begin(false))
				fputs("\n", outFile);
			fputs((iter->getOutput()).c_str(), outFile);

			if (!background_script_required && (iter->name == "GRUB_MENU_PICTURE" || iter->name == "GRUB_COLOR_NORMAL" || iter->name == "GRUB_COLOR_HIGHLIGHT"))
				background_script_required = true;
		}
		fclose(outFile);
		if (background_script_required){
			mkdir((env.cfg_dir_prefix+"/usr/share/desktop-base").c_str(), 0755);
			FILE* bgScriptFile = fopen((env.cfg_dir_prefix+"/usr/share/desktop-base/grub_background.sh").c_str(), "w");
			chmod((env.cfg_dir_prefix+"/usr/share/desktop-base/grub_background.sh").c_str(), 0755);
			fputs(background_script, bgScriptFile);
			fclose(bgScriptFile);
		}
		this->_reloadRequired = false;
		return true;
	}
	else
		return false;
}

void SettingsManagerDataStore::clear(){
	this->settings.clear();
}
