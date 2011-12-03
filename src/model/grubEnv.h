#ifndef GRUBENV_INCLUDED
#define GRUBENV_INCLUDED

#include <string>
#include <cstdio>
#include "mountTable.h"
#include <cstdlib>
#include <dirent.h>
#include "../presenter/commonClass.h"

struct GrubEnv : public CommonClass {
	enum Mode {
		GRUB_MODE,
		BURG_MODE
	};
	GrubEnv();
	bool init(GrubEnv::Mode mode, std::string const& dir_prefix);
	bool check_cmd(std::string const& cmd, std::string const& cmd_prefix = "") const;
	bool check_dir(std::string const& dir) const;
	std::string getRootDevice();
	std::string cfg_dir, cfg_dir_noprefix, mkconfig_cmd, cfg_dir_prefix, update_cmd, install_cmd, output_config_file, output_config_dir, settings_file;
	bool burgMode;
	std::list<GrubEnv::Mode> getAvailableModes();
};

#endif
