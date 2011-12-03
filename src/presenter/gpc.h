#ifndef GTK_CLIENT_INCLUDED
#define GTK_CLIENT_INCLUDED

#include "../model/grublistCfg.h"
#include "../interface/grublistCfgDlg.h"
#include <libintl.h>
#include <locale.h>
#include <sstream>
#include "../config.h"

#include "../model/grubEnv.h"

#include "../model/mountTable.h"
#include "../interface/partitionChooser.h"

#include "../model/grubInstaller.h"
#include "../interface/grubInstallDlg.h"

#include "../interface/scriptAddDlg.h"

#include "../model/grublistCfg.h"
#include "../interface/partitionChooser.h"
#include "../interface/settingsDlg.h"
#include "../model/fbResolutionsGetter.h"
#include "../interface/aboutDialog.h"
#include "../model/deviceDataList.h"
#include "../interface/threadController.h"

#include "commonClass.h"

class GPC : public CommonClass {
	GrubEnv& env;
	GrublistCfg* grublistCfg;
	GrublistCfgDlg* listCfgDlg;
	SettingsDlg* settingsDlg;
	SettingsManagerDataStore* settings;
	SettingsManagerDataStore* settingsOnDisk;
	GrubInstaller* installer;
	GrubInstallDlg* installDlg;
	ScriptAddDlg* scriptAddDlg;
	PartitionChooser* partitionChooser;
	GrublistCfg* savedListCfg;
	FbResolutionsGetter* fbResolutionsGetter;
	DeviceDataList* deviceDataList;
	MountTable* mountTable;
	AboutDialog* aboutDialog;
	ThreadController* threadController;

	bool config_has_been_different_on_startup_but_unsaved;
	bool modificationsUnsaved;
	bool quit_requested;
	bool is_loading;
	int activeThreadCount;
	GrublistCfg::Exception thrownException;

	void _rAppendRule(Rule& rule, Rule* parentRule = NULL);

public:
	enum Exception {
		INCOMPLETE
	};
	void setListCfg(GrublistCfg& grublistCfg);
	void setListCfgDlg(GrublistCfgDlg& listCfgDlg);
	void setSettingsDialog(SettingsDlg& settingsDlg);
	void setSettingsManager(SettingsManagerDataStore& settings);
	void setSettingsBuffer(SettingsManagerDataStore& settings);
	void setInstaller(GrubInstaller& installer);
	void setInstallDlg(GrubInstallDlg& installDlg);
	void setScriptAddDlg(ScriptAddDlg& scriptAddDlg);
	void setPartitionChooser(PartitionChooser& partitionChooser);
	void setSavedListCfg(GrublistCfg& savedListCfg);
	void setFbResolutionsGetter(FbResolutionsGetter& fbResolutionsGetter);
	void setDeviceDataList(DeviceDataList& deviceDataList);
	void setMountTable(MountTable& mountTable);
	void setAboutDialog(AboutDialog& aboutDialog);
	void setThreadController(ThreadController& threadController);

	ThreadController& getThreadController();
	FbResolutionsGetter& getFbResolutionsGetter();

	void init();
	void init(GrubEnv::Mode mode);
	void hidePartitionChooserQuestion();
	void showPartitionChooser();
	void handleCancelResponse();

	void showSettingsDlg();
	void reload();
	void load(bool preserveConfig = false);
	void save();
	void save_thread();
	GPC(GrubEnv& env);
	
	void renameEntry(Rule* rule, std::string const& newName);
	void reset();
	void initRootSelector();
	
	void showInstallDialog();
	void installGrub(std::string device);
	void showMessageGrubInstallCompleted(std::string const& msg);
	
	void showScriptAddDlg();
	void addScriptFromScriptAddDlg();
	void updateScriptAddDlgPreview();
	
	void removeProxy(Proxy* p);
	
	void syncListView_load();
	void syncListView_save();
	void die();
	void activateSettingsBtn();
	void updateSettingsDlg();
	
	void quit(bool force = false);
	
	void syncProxyState(void* proxy);
	void syncRuleState(Rule* entry);
	void syncRuleName(Rule* entry);
	void updateScriptEntry(Proxy* proxy);
	void moveRule(void* rule, int direction);
	void swapProxies(Proxy* a, Proxy* b);
	
	void showRuleInfo(Rule* rule);
	void showProxyInfo(Proxy* proxy);

	void showAboutDialog();

	void updateSettingsDlgResolutionList_dispatched();

	void syncSettings();
	void updateDefaultSetting();
	void updateCustomSetting(std::string const& name);
	void addNewCustomSettingRow();
	void removeCustomSettingRow(std::string const& name);
	void updateShowMenuSetting();
	void updateOsProberSetting();
	void updateTimeoutSetting();
	void updateKernalParams();
	void updateGenerateRecoverySetting();
	void updateCustomResolution();
	void updateColorSettings();
	void updateBackgroundImage();
	void updateUseCustomResolution();
	void removeBackgroundImage();
	void hideSettingsDialog();

	void mountRootFs();
	void umountRootFs();
	void cancelPartitionChooser();
	void applyPartitionChooser();
	void mountSubmountpoint(std::string const& submountpoint);
	void umountSubmountpoint(std::string const& submountpoint);
	void readPartitionInfo();
	void generateSubmountpointSelection(std::string const& prefix);
};

#endif
