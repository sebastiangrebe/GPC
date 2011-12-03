#ifndef DEVICE_DATALIST_INCLUDED
#define DEVICE_DATALIST_INCLUDED
#include <map>
#include <cstdio>
#include <string>
#include "../presenter/commonClass.h"

class DeviceDataList : public std::map<std::string, std::map<std::string, std::string> >, public CommonClass {
public:
	DeviceDataList(FILE* blkidOutput);
	DeviceDataList();
	void loadData(FILE* blkidOutput);
	void clear();
};
#endif
