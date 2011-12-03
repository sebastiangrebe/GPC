#include "libproxyscript_parser.h"

ProxyScriptData::ProxyScriptData(FILE* fpProxyScript)
	: is_valid(false)
{
	load(fpProxyScript);
}

bool ProxyScriptData::is_proxyscript(FILE* proxy_fp){
	int c;
	while ((c = fgetc(proxy_fp)) != EOF){
		if (c == '\n')
			break;
	}
	std::string textBefore = "#THIS IS A GRUB PROXY SCRIPT";
	bool match_error = false;
	for (int i = 0; i < textBefore.length() && (c = fgetc(proxy_fp)) != EOF; i++){
		if (c != textBefore[i]){
			match_error = true; 
			break;
		}
	}
	
	if (!match_error){
		while ((c = fgetc(proxy_fp)) != EOF){
			if (c == '\n')
				break;
		}
	}
	return !match_error;
}

bool ProxyScriptData::is_proxyscript(std::string const& filePath){
	bool result = false;
	FILE* f = fopen(filePath.c_str(), "r");
	if (f){
		result = ProxyScriptData::is_proxyscript(f);
		fclose(f);
	}
	return result;
}

bool ProxyScriptData::load(FILE* fpProxyScript){
	this->scriptCmd = "";
	this->proxyCmd = "";
	this->ruleString = "";
	
	if (ProxyScriptData::is_proxyscript(fpProxyScript)){
		int c;
		bool is_begin_of_row = true, is_comment = false;
		int parseStep = 0;
		bool inQuotes = false;
		bool success = false;
		while ((c = fgetc(fpProxyScript)) != EOF && !success){
			if (is_begin_of_row && c == '#'){
				is_comment = true;
				is_begin_of_row = false;
			}
			else if (is_comment && c == '\n'){
				is_comment = false;
			}
			else if (!is_comment) {
				if (parseStep == 0){
					if (this->scriptCmd.length() == 0 && inQuotes == false && c == '\''){
						inQuotes = true;
					}
					else if (!inQuotes && c != ' ' || inQuotes && c != '\''){
						this->scriptCmd += char(c);
					}
					else {
						inQuotes = false;
						parseStep = 1;
					}
				}
				else if (parseStep == 1 && c != ' ' && c != '|'){
					parseStep = 2;
				}
				if (parseStep == 2){
					if (c != ' ')
						this->proxyCmd += char(c);
					else
						parseStep = 3;
				}
				if (parseStep == 3){
					if (c == '"' && !inQuotes)
						inQuotes = true;
					else if ((c == '"' || c == '\\') && inQuotes && (this->ruleString.length() > 0 && this->ruleString[this->ruleString.length()-1] == '\\'))
						this->ruleString[this->ruleString.length()-1] = char(c);
					else if (c == '"' && inQuotes){
						if (this->scriptCmd != "" && this->proxyCmd != "" && this->ruleString != ""){
							success = true;
						}
						else
							parseStep = 0;
					}
					else {
						this->ruleString += char(c);
					}
				}
			}
		
			if (c == '\n')
				is_begin_of_row = true;
		}
		this->is_valid = true;
	}
}

ProxyScriptData::operator bool(){
	return is_valid;
}
