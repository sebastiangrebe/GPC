#include "gpc.h"

GPC::GPC(GrubEnv& env)
	: grublistCfg(NULL), listCfgDlg(NULL), settingsDlg(NULL), settings(NULL),
	  installer(NULL), installDlg(NULL), settingsOnDisk(NULL), scriptAddDlg(NULL),
	  partitionChooser(NULL), savedListCfg(NULL),
	  fbResolutionsGetter(NULL), deviceDataList(NULL),
	  mountTable(NULL), aboutDialog(NULL),
	 env(env), config_has_been_different_on_startup_but_unsaved(false),
	 modificationsUnsaved(false), quit_requested(false), activeThreadCount(0),
	 is_loading(false)
{
}


void GPC::setListCfg(GrublistCfg& grublistCfg){
	this->grublistCfg = &grublistCfg;
}
void GPC::setListCfgDlg(GrublistCfgDlg& listCfgDlg){
	this->listCfgDlg = &listCfgDlg;
}

void GPC::setSettingsDialog(SettingsDlg& settingsDlg){
	this->settingsDlg = &settingsDlg;
}

void GPC::setSettingsManager(SettingsManagerDataStore& settings){
	this->settings = &settings;
}

void GPC::setSettingsBuffer(SettingsManagerDataStore& settings){
	this->settingsOnDisk = &settings;
}

void GPC::setInstaller(GrubInstaller& installer){
	this->installer = &installer;
}
void GPC::setInstallDlg(GrubInstallDlg& installDlg){
	this->installDlg = &installDlg;
}
void GPC::setScriptAddDlg(ScriptAddDlg& scriptAddDlg){
	this->scriptAddDlg = &scriptAddDlg;
}

void GPC::setSavedListCfg(GrublistCfg& savedListCfg){
	this->savedListCfg = &savedListCfg;
}

void GPC::setPartitionChooser(PartitionChooser& partitionChooser){
	this->partitionChooser = &partitionChooser;
}

void GPC::setFbResolutionsGetter(FbResolutionsGetter& fbResolutionsGetter){
	this->fbResolutionsGetter = &fbResolutionsGetter;
}

void GPC::setDeviceDataList(DeviceDataList& deviceDataList){
	this->deviceDataList = &deviceDataList;
}

void GPC::setMountTable(MountTable& mountTable){
	this->mountTable = &mountTable;
}

void GPC::setAboutDialog(AboutDialog& aboutDialog){
	this->aboutDialog = &aboutDialog;
}

void GPC::setThreadController(ThreadController& threadController) {
	this->threadController = &threadController;
}

ThreadController& GPC::getThreadController() {
	if (this->threadController == NULL) {
		throw INCOMPLETE;
	}
	return *this->threadController;
}

FbResolutionsGetter& GPC::getFbResolutionsGetter() {
	return *this->fbResolutionsGetter;
}

void GPC::updateSettingsDlg(){
	std::list<EntryTitleListItem> entryTitles = this->grublistCfg->proxies.generateEntryTitleList();
	this->settingsDlg->clearDefaultEntryChooser();
	for (std::list<EntryTitleListItem>::iterator iter = entryTitles.begin(); iter != entryTitles.end(); iter++)
		this->settingsDlg->addEntryToDefaultEntryChooser(iter->labelPathValue, iter->labelPathLabel, iter->numericPathValue, iter->numericPathLabel);


	this->syncSettings();
}

void GPC::init(){
	if   ( !grublistCfg
		or !listCfgDlg
		or !settingsDlg
		or !settings
		or !settingsOnDisk
		or !installer
		or !installDlg
		or !scriptAddDlg
		or !savedListCfg
		or !fbResolutionsGetter
		or !deviceDataList
		or !mountTable
		or !aboutDialog
	) {
		throw INCOMPLETE;
	}
	this->log("initializing (w/o specified bootloader type)…", Logger::IMPORTANT_EVENT);
	savedListCfg->verbose = false;

	this->log("reading partition info…", Logger::EVENT);
	FILE* blkidProc = popen("blkid", "r");
	if (blkidProc){
		deviceDataList->clear();
		deviceDataList->loadData(blkidProc);
		pclose(blkidProc);
	}

	mountTable->loadData("");
	mountTable->loadData(PARTCHOOSER_MOUNTPOINT);


	this->log("Loading Framebuffer resolutions (background process)", Logger::EVENT);
	this->getThreadController().startFramebufferResolutionLoader();

	this->log("Finding out if this is a live CD", Logger::EVENT);
	if (mountTable->getEntryByMountpoint("").isLiveCdFs() && env.cfg_dir_prefix == ""){
		this->log("is live CD", Logger::INFO);
		partitionChooser->setIsStartedManually(false);
		this->initRootSelector();
		partitionChooser->show();
	} else {
		this->log("running on an installed system", Logger::INFO);
		std::list<GrubEnv::Mode> modes = this->env.getAvailableModes();
		if (modes.size() == 2)
			this->listCfgDlg->showBurgSwitcher();
		else if (modes.size() == 1)
			this->init(modes.front());
		else if (modes.size() == 0)
			this->listCfgDlg->showPartitionChooserQuestion();
	}
}

void GPC::init(GrubEnv::Mode mode){
	this->log("initializing (w/ specified bootloader type)…", Logger::IMPORTANT_EVENT);
	this->env.init(mode, env.cfg_dir_prefix);
	this->listCfgDlg->setLockState(1|4|8);
	this->listCfgDlg->setIsBurgMode(mode == GrubEnv::BURG_MODE);
	this->listCfgDlg->show();
	this->listCfgDlg->hideBurgSwitcher();

	this->log("Checking if the config directory is clean", Logger::EVENT);
	if (this->grublistCfg->cfgDirIsClean() == false) {
		this->log("cleaning up config dir", Logger::IMPORTANT_EVENT);
		this->grublistCfg->cleanupCfgDir();
	}

	this->log("loading configuration", Logger::IMPORTANT_EVENT);
	this->getThreadController().startLoadThread(false);
}

void GPC::hidePartitionChooserQuestion(){
	this->listCfgDlg->hidePartitionChooserQuestion();
}

void GPC::showPartitionChooser(){
	partitionChooser->setIsStartedManually(true);
	this->initRootSelector();
	this->partitionChooser->show();
}

void GPC::handleCancelResponse(){
	if (!listCfgDlg->isVisible())
		this->getThreadController().stopApplication();
}

void GPC::showSettingsDlg(){
	this->settingsDlg->show(env.burgMode);
}

void GPC::reload(){
	this->syncSettings();
	this->listCfgDlg->setLockState(1|4|8);
	this->getThreadController().startLoadThread(true);
}

void GPC::load(bool preserveConfig){
	if (!is_loading){
		this->log(std::string("loading - preserveConfig: ") + (preserveConfig ? "yes" : "no"), Logger::IMPORTANT_EVENT);
		is_loading = true;
		this->activeThreadCount++;

		if (!preserveConfig){
			this->log("unsetting saved config", Logger::EVENT);
			this->grublistCfg->reset();
			this->savedListCfg->reset();
			this->log("loading settings", Logger::IMPORTANT_EVENT);
			this->settings->load();
			this->getThreadController().enableSettings();
		} else {
			this->log("switching settings", Logger::IMPORTANT_EVENT);
			this->settingsOnDisk->load();
			this->settings->save();
		}

		try {
			this->log("loading grub list", Logger::IMPORTANT_EVENT);
			this->grublistCfg->load(preserveConfig);
			this->log("grub list completely loaded", Logger::IMPORTANT_EVENT);
		} catch (GrublistCfg::Exception e){
			this->log("error while loading the grub list", Logger::ERROR);
			this->thrownException = e;
			this->getThreadController().showThreadDiedError();
			return; 
		}
	
		if (!preserveConfig){
			this->log("loading saved grub list", Logger::IMPORTANT_EVENT);
			if (this->savedListCfg->loadStaticCfg()) {
				this->config_has_been_different_on_startup_but_unsaved = !this->grublistCfg->compare(*this->savedListCfg);
			} else {
				this->log("saved grub list not found", Logger::WARNING);
				this->config_has_been_different_on_startup_but_unsaved = false;
			}
		}
		if (preserveConfig){
			this->log("restoring settings", Logger::IMPORTANT_EVENT);
			this->settingsOnDisk->save();
		}
		this->activeThreadCount--;
		this->is_loading = false;
	} else {
		this->log("ignoring load request (only one load thread allowed at the same time)", Logger::WARNING);
	}
}

void GPC::save(){
	this->config_has_been_different_on_startup_but_unsaved = false;
	this->modificationsUnsaved = false; 

	this->listCfgDlg->setLockState(1|4|8);
	this->activeThreadCount++; 
	this->getThreadController().startSaveThread();
}

void GPC::save_thread(){
	this->log("writing settings file", Logger::IMPORTANT_EVENT);
	this->settings->save();
	this->log("writing grub list configuration", Logger::IMPORTANT_EVENT);
	this->grublistCfg->save();
	this->activeThreadCount--;
}

void GPC::renameEntry(Rule* rule, std::string const& newName){
	if (this->settings->getValue("GRUB_DEFAULT") == rule->outputName)
		this->settings->setValue("GRUB_DEFAULT", newName);
	this->grublistCfg->renameRule(rule, newName);
}

void GPC::reset(){
	this->grublistCfg->reset();
	this->settings->clear();
}


void GPC::showAboutDialog(){
	this->aboutDialog->show();
}


void GPC::initRootSelector(){
	if (mountTable->getEntryByMountpoint(PARTCHOOSER_MOUNTPOINT)){
		partitionChooser->setIsMounted(true);
		this->generateSubmountpointSelection(PARTCHOOSER_MOUNTPOINT);
	}
	else
		partitionChooser->setIsMounted(false);
	this->readPartitionInfo();
}

void GPC::readPartitionInfo(){
	this->partitionChooser->clearPartitionSelector();
	for (DeviceDataList::iterator iter = deviceDataList->begin(); iter != deviceDataList->end(); iter++){
		this->partitionChooser->addPartitionSelectorItem(iter->first, iter->second["TYPE"], iter->second["LABEL"]);
	}
}

void GPC::generateSubmountpointSelection(std::string const& prefix){
	this->partitionChooser->removeAllSubmountpoints();

	for (MountTable::const_iterator iter = mountTable->begin(); iter != mountTable->end(); iter++){
		if (iter->mountpoint.length() > prefix.length()
		 && iter->mountpoint != prefix + "/dev"
		 && iter->mountpoint != prefix + "/proc"
		 && iter->mountpoint != prefix + "/sys"
		) {
			this->partitionChooser->addSubmountpoint(iter->mountpoint.substr(prefix.length()), iter->isMounted);
		}
	}
}

void GPC::mountRootFs(){
	std::string selectedDevice = this->partitionChooser->getSelectedDevice();
	partitionChooser->setIsMounted(true);
	mkdir(PARTCHOOSER_MOUNTPOINT, 0755);
	try {
		mountTable->clear(PARTCHOOSER_MOUNTPOINT);
		mountTable->mountRootFs(selectedDevice, PARTCHOOSER_MOUNTPOINT);
		this->generateSubmountpointSelection(PARTCHOOSER_MOUNTPOINT);
	}
	catch (Mountpoint::Exception e) {
		if (e == Mountpoint::MOUNT_FAILED){
			this->partitionChooser->showErrorMessage(PartitionChooser::MOUNT_FAILED);
			partitionChooser->setIsMounted(false);
		}
	}
	catch (MountTable::Exception e) {
		if (e == MountTable::MOUNT_ERR_NO_FSTAB){
			this->partitionChooser->showErrorMessage(PartitionChooser::MOUNT_ERR_NO_FSTAB);
			mountTable->getEntryByMountpoint(PARTCHOOSER_MOUNTPOINT).umount();
			partitionChooser->setIsMounted(false);
		}
	}
}


void GPC::umountRootFs(){
	try {
		this->mountTable->umountAll(PARTCHOOSER_MOUNTPOINT);
		this->mountTable->clear(PARTCHOOSER_MOUNTPOINT);
		partitionChooser->setIsMounted(false);
	}
	catch (Mountpoint::Exception e){
		if (e == Mountpoint::UMOUNT_FAILED)
			this->partitionChooser->showErrorMessage(PartitionChooser::UMOUNT_FAILED);
	}

	this->reset();
	this->syncListView_load();
	listCfgDlg->setLockState(1|2|8);
	this->syncSettings();
}


void GPC::cancelPartitionChooser(){
	this->partitionChooser->hide();
	this->handleCancelResponse();
}


void GPC::applyPartitionChooser(){
	this->partitionChooser->hide();
	listCfgDlg->setLockState(1|2|8);
	this->syncSettings();
	settingsDlg->hide();
	scriptAddDlg->hide();
	this->env.cfg_dir_prefix = PARTCHOOSER_MOUNTPOINT;
	this->init();
}

void GPC::mountSubmountpoint(std::string const& submountpoint){
	try {
		this->mountTable->getEntryRefByMountpoint(PARTCHOOSER_MOUNTPOINT + submountpoint).mount();
	}
	catch (Mountpoint::Exception e){
		if (e == Mountpoint::MOUNT_FAILED){
			this->partitionChooser->showErrorMessage(PartitionChooser::SUB_MOUNT_FAILED);
		}
		this->partitionChooser->setSubmountpointSelectionState(submountpoint, false);
	}
}

void GPC::umountSubmountpoint(std::string const& submountpoint){
	try {
		this->mountTable->getEntryRefByMountpoint(PARTCHOOSER_MOUNTPOINT + submountpoint).umount();
	}
	catch (Mountpoint::Exception e){
		if (e == Mountpoint::UMOUNT_FAILED){
			this->partitionChooser->showErrorMessage(PartitionChooser::SUB_UMOUNT_FAILED);
		}
		this->partitionChooser->setSubmountpointSelectionState(submountpoint, true);
	}
}

void GPC::showInstallDialog(){
	installDlg->show();
}

void GPC::installGrub(std::string device){
	this->activeThreadCount++;
	installer->threadable_install(device);
	this->activeThreadCount--;
	if (this->activeThreadCount == 0 && this->quit_requested)
		this->quit(true);
}

void GPC::showMessageGrubInstallCompleted(std::string const& msg){
	installDlg->showMessageGrubInstallCompleted(msg);
}

void GPC::showScriptAddDlg(){
	if (grublistCfg->repository.size() > 0){
		scriptAddDlg->clear();
		for (Repository::iterator iter = grublistCfg->repository.begin(); iter != grublistCfg->repository.end(); iter++){
			scriptAddDlg->addItem(iter->name);
		}
	}
	else
		this->listCfgDlg->showErrorMessage(gettext("No script found"));
	
	
	scriptAddDlg->show();
}

void GPC::addScriptFromScriptAddDlg(){
	Script* script = grublistCfg->repository.getNthScript(scriptAddDlg->getSelectedEntryIndex());
	grublistCfg->proxies.push_back(Proxy(*script));
	grublistCfg->renumerate();
	
	this->syncListView_load();
	
	this->modificationsUnsaved = true;
}

void GPC::updateScriptAddDlgPreview(){
	scriptAddDlg->clearPreview();
	Script* selectedScript = grublistCfg->repository.getNthScript(scriptAddDlg->getSelectedEntryIndex());
	if (selectedScript){
		for (Script::iterator iter = selectedScript->begin(); iter != selectedScript->end(); iter++)
			scriptAddDlg->addToPreview(iter->name);
	}
}

void GPC::removeProxy(Proxy* p){
	this->grublistCfg->proxies.deleteProxy(p);
	this->listCfgDlg->removeProxy(p);
	this->syncListView_load();
	this->modificationsUnsaved = true;
}

void GPC::_rAppendRule(Rule& rule, Rule* parentRule){
	bool is_other_entries_ph = rule.type == Rule::OTHER_ENTRIES_PLACEHOLDER;
	if (rule.dataSource || is_other_entries_ph){
		std::string name;
		if (is_other_entries_ph) {
			try {
				if (rule.dataSource == NULL) {
					throw 1;
				}
				Proxy* proxy = this->grublistCfg->proxies.getProxyByRule(&rule);
				Rule* parentRule = proxy->getRuleByEntry(*rule.dataSource, proxy->rules, Rule::NORMAL);
				if (parentRule) {
					name = this->listCfgDlg->createNewEntriesPlaceholderString(parentRule->outputName);
				} else {
					throw 1;
				}
			} catch (...) {
				name = this->listCfgDlg->createNewEntriesPlaceholderString();
			}
		} else {
			name = rule.outputName;
		}
		bool isSubmenu = rule.type == Rule::NORMAL && rule.dataSource && rule.dataSource->type == Entry::SUBMENU;
		this->listCfgDlg->appendEntry(name, rule.isVisible, &rule, !is_other_entries_ph, isSubmenu, parentRule);

		for (std::list<Rule>::iterator subruleIter = rule.subRules.begin(); subruleIter != rule.subRules.end(); subruleIter++) {
			this->_rAppendRule(*subruleIter, &rule);
		}
	}
}

void GPC::syncListView_load() {
	this->log("running GPC::syncListView_load", Logger::INFO);
	this->listCfgDlg->setLockState(1|4);
	double progress = this->grublistCfg->getProgress();
	if (progress != 1) {
		this->listCfgDlg->setProgress(progress);
		this->listCfgDlg->setStatusText(gettext("loading configuration…"));
	} else {
		if (this->quit_requested) {
			this->quit(true);
		}
		this->listCfgDlg->hideProgressBar();
		this->listCfgDlg->setStatusText("");
	}
	
	if (this->grublistCfg->lock_if_free()) {
		this->listCfgDlg->clear();
	
		for (std::list<Proxy>::iterator iter = this->grublistCfg->proxies.begin(); iter != this->grublistCfg->proxies.end(); iter++){
			std::string name = iter->getScriptName() + (this->grublistCfg && iter->dataSource && (progress != 1 && iter->dataSource->fileName != iter->fileName || progress == 1 && grublistCfg->proxies.proxyRequired(*iter->dataSource)) ? gettext(" (custom)") : "");
			this->listCfgDlg->appendScript(name, iter->isExecutable(), &(*iter));
			for (std::list<Rule>::iterator ruleIter = iter->rules.begin(); ruleIter != iter->rules.end(); ruleIter++){
				this->_rAppendRule(*ruleIter);
			}
		}
		this->grublistCfg->unlock();
	}

	if (progress == 1){
		this->updateSettingsDlg();
		this->listCfgDlg->setLockState(0);
	}
	this->log("GPC::syncListView_load completed", Logger::INFO);
}

void GPC::syncListView_save(){
	this->log("running GPC::syncListView_save", Logger::INFO);
	this->listCfgDlg->progress_pulse();
	if (this->grublistCfg->getProgress() == 1){
		if (this->grublistCfg->error_proxy_not_found){
			this->listCfgDlg->showProxyNotFoundMessage();
			this->grublistCfg->error_proxy_not_found = false;
		}
		if (this->quit_requested)
			this->quit(true);
		
		this->listCfgDlg->setLockState(0);
		
		this->listCfgDlg->hideProgressBar();
		this->listCfgDlg->setStatusText(gettext("Configuration has been saved"));
	}
	else {
		this->listCfgDlg->setStatusText(gettext("updating configuration"));
	}
	this->log("GPC::syncListView_save completed", Logger::INFO);
}

void GPC::die(){
	switch (this->thrownException){
		case GrublistCfg::GRUB_CFG_DIR_NOT_FOUND:
			this->listCfgDlg->showErrorMessage(
					this->env.cfg_dir+gettext(" not found. Is grub2 installed?")
			);
			break;
		case GrublistCfg::GRUB_CMD_EXEC_FAILED:
			this->listCfgDlg->showErrorMessage(
					this->env.mkconfig_cmd + gettext(" couldn't be executed successfully. You must run this as root!")
			);
			break;
	}
	this->quit(true);
}

void GPC::activateSettingsBtn(){
	this->syncSettings();
	this->listCfgDlg->setLockState(1);
}

void GPC::quit(bool force){
	if (force){
		if (this->mountTable->getEntryByMountpoint(PARTCHOOSER_MOUNTPOINT))
			this->mountTable->umountAll(PARTCHOOSER_MOUNTPOINT);
		this->getThreadController().stopApplication();
	}
	else {
		int dlgResponse = this->listCfgDlg->showExitConfirmDialog(this->config_has_been_different_on_startup_but_unsaved*2 + this->modificationsUnsaved);
		if (dlgResponse == 1){
			this->save(); 
		}

		if (dlgResponse != 0){
			if (this->activeThreadCount != 0){
				this->quit_requested = true;
				this->grublistCfg->cancelThreads();
			}
			else {
				this->quit(true); 
			}
		}
	}
}

void GPC::syncProxyState(void* proxy){
	((Proxy*)proxy)->set_isExecutable(this->listCfgDlg->getProxyState(proxy));
	this->listCfgDlg->setProxyState(proxy, this->listCfgDlg->getProxyState(proxy));
	this->modificationsUnsaved = true;
	this->updateSettingsDlg();
}

void GPC::syncRuleState(Rule* entry){
	entry->isVisible = this->listCfgDlg->getRuleState(entry);
	this->listCfgDlg->setRuleState(entry, this->listCfgDlg->getRuleState(entry));
	this->modificationsUnsaved = true;
	this->updateScriptEntry(this->grublistCfg->proxies.getProxyByRule(entry));
	this->updateSettingsDlg();
}

void GPC::syncRuleName(Rule* entry){
	std::string oldName = entry->outputName;
	std::string newName = this->listCfgDlg->getRuleName(entry);
	if (newName == ""){
		this->listCfgDlg->showErrorMessage(gettext("Name the Entry"));
		this->listCfgDlg->setRuleName(entry, oldName);
		this->updateScriptEntry(this->grublistCfg->proxies.getProxyByRule(entry));
	}
	else {
		this->renameEntry(entry, newName);
	}
	this->modificationsUnsaved = true;
}

void GPC::updateScriptEntry(Proxy* proxy){
	if (proxy->dataSource){ 
		std::string name = proxy->dataSource->name;
		this->listCfgDlg->setProxyName(proxy, name, false);
		if (this->grublistCfg->proxies.proxyRequired(*proxy->dataSource)){
			this->listCfgDlg->setProxyName(proxy, name, true);
		}
	}
}

void GPC::moveRule(void* rule, int direction){
	Proxy* parentProxy = this->grublistCfg->proxies.getProxyByRule((Rule*)rule);
	try {
		Rule* newRule = &this->grublistCfg->moveRule((Rule*)rule, direction);

		this->syncListView_load();
		this->listCfgDlg->selectRule(newRule);
		this->updateScriptEntry(parentProxy);
		this->modificationsUnsaved = true;
	} catch (GrublistCfg::Exception e) {
		if (e == GrublistCfg::NO_MOVE_TARGET_FOUND)
			this->listCfgDlg->showErrorMessage(gettext("cannot move this entry"));
		else
			throw e;
	}
}

void GPC::swapProxies(Proxy* a, Proxy* b){
	grublistCfg->swapProxies(a,b);
	this->listCfgDlg->swapProxies(a,b);
	this->modificationsUnsaved = true;
}


void GPC::showRuleInfo(Rule* rule){
	if (rule && rule->dataSource)
		this->listCfgDlg->setDefaultTitleStatusText(rule->getEntryName());
	else
		this->listCfgDlg->setStatusText("");
}

void GPC::showProxyInfo(Proxy* proxy){
	this->listCfgDlg->setStatusText("");
}

void GPC::updateSettingsDlgResolutionList_dispatched(){
	const std::list<std::string>& data = this->fbResolutionsGetter->getData();
	this->settingsDlg->clearResolutionChooser();
	for (std::list<std::string>::const_iterator iter = data.begin(); iter != data.end(); iter++)
		this->settingsDlg->addResolution(*iter);
}

void GPC::syncSettings(){
	std::string sel = this->settingsDlg->getSelectedCustomOption();
	this->settingsDlg->removeAllSettingRows();
	for (std::list<SettingRow>::iterator iter = this->settings->begin(); iter != this->settings->end(); this->settings->iter_to_next_setting(iter)){
		this->settingsDlg->addCustomOption(iter->isActive, iter->name, iter->value);
	}
	this->settingsDlg->selectCustomOption(sel);
	std::string defEntry = this->settings->getValue("GRUB_DEFAULT");
	if (defEntry == "saved"){
		this->settingsDlg->setActiveDefEntryOption(SettingsDlg::DEF_ENTRY_SAVED);
	}
	else {
		this->settingsDlg->setActiveDefEntryOption(SettingsDlg::DEF_ENTRY_PREDEFINED);
		this->settingsDlg->setDefEntry(defEntry);
	}

	this->settingsDlg->setShowMenuCheckboxState(!this->settings->isActive("GRUB_HIDDEN_TIMEOUT", true));
	this->settingsDlg->setOsProberCheckboxState(!this->settings->isActive("GRUB_DISABLE_OS_PROBER", true));

	std::string timeoutStr;
	if (this->settingsDlg->getShowMenuCheckboxState())
		timeoutStr = this->settings->getValue("GRUB_TIMEOUT");
	else
		timeoutStr = this->settings->getValue("GRUB_HIDDEN_TIMEOUT");

	if (timeoutStr == "" || timeoutStr.find_first_not_of("0123456789") != -1) {
		timeoutStr = "10";
	}
	std::istringstream in(timeoutStr);
	int timeout;
	in >> timeout;
	this->settingsDlg->setTimeoutValue(timeout);

	this->settingsDlg->setKernelParams(this->settings->getValue("GRUB_CMDLINE_LINUX_DEFAULT"));
	this->settingsDlg->setRecoveryCheckboxState(!this->settings->isActive("GRUB_DISABLE_LINUX_RECOVERY", true));

	this->settingsDlg->setResolutionCheckboxState(this->settings->isActive("GRUB_GFXMODE", true));
	this->settingsDlg->setResolution(this->settings->getValue("GRUB_GFXMODE"));

	std::string nColor = this->settings->getValue("GRUB_COLOR_NORMAL");
	std::string hColor = this->settings->getValue("GRUB_COLOR_HIGHLIGHT");
	if (nColor != ""){
		this->settingsDlg->getColorChooser(SettingsDlg::COLOR_CHOOSER_DEFAULT_FONT).selectColor(nColor.substr(0, nColor.find('/')));
		this->settingsDlg->getColorChooser(SettingsDlg::COLOR_CHOOSER_DEFAULT_BACKGROUND).selectColor(nColor.substr(nColor.find('/')+1));
	}
	else {
		this->settingsDlg->getColorChooser(SettingsDlg::COLOR_CHOOSER_DEFAULT_FONT).selectColor("light-gray");
		this->settingsDlg->getColorChooser(SettingsDlg::COLOR_CHOOSER_DEFAULT_BACKGROUND).selectColor("black");
	}
	if (hColor != ""){
		this->settingsDlg->getColorChooser(SettingsDlg::COLOR_CHOOSER_HIGHLIGHT_FONT).selectColor(hColor.substr(0, hColor.find('/')));
		this->settingsDlg->getColorChooser(SettingsDlg::COLOR_CHOOSER_HIGHLIGHT_BACKGROUND).selectColor(hColor.substr(hColor.find('/')+1));
	}
	else {
		this->settingsDlg->getColorChooser(SettingsDlg::COLOR_CHOOSER_HIGHLIGHT_FONT).selectColor("magenta");
		this->settingsDlg->getColorChooser(SettingsDlg::COLOR_CHOOSER_HIGHLIGHT_BACKGROUND).selectColor("black");
	}

	std::string menuPicturePath = this->settings->getValue("GRUB_MENU_PICTURE");
	bool menuPicIsInGrubDir = false;
	if (menuPicturePath != "" && menuPicturePath[0] != '/'){
		menuPicturePath = env.output_config_dir + "/" + menuPicturePath;
		menuPicIsInGrubDir = true;
	}

	if (this->settings->isActive("GRUB_MENU_PICTURE") && menuPicturePath != ""){
		this->settingsDlg->setBackgroundImagePreviewPath(menuPicturePath, menuPicIsInGrubDir);
	}
	else {
		this->settingsDlg->setBackgroundImagePreviewPath("", menuPicIsInGrubDir);
	}
}

void GPC::updateDefaultSetting(){
	if (this->settingsDlg->getActiveDefEntryOption() == SettingsDlg::DEF_ENTRY_SAVED){
		this->settings->setValue("GRUB_DEFAULT", "saved");
		this->settings->setValue("GRUB_SAVEDEFAULT", "true");
		this->settings->setIsActive("GRUB_SAVEDEFAULT", true);
	}
	else {
		this->settings->setValue("GRUB_DEFAULT", this->settingsDlg->getSelectedDefaultGrubValue());
		this->settings->setValue("GRUB_SAVEDEFAULT", "false");
	}
	this->syncSettings();
	this->modificationsUnsaved = true;
}

void GPC::updateCustomSetting(std::string const& name){
	SettingsDlg::CustomOption c = this->settingsDlg->getCustomOption(name);
	this->settings->renameItem(c.old_name, c.name);
	this->settings->setValue(c.name, c.value);
	this->settings->setIsActive(c.name, c.isActive);
	this->syncSettings();
	this->modificationsUnsaved = true;
}

void GPC::addNewCustomSettingRow(){
	std::string newSettingName = this->settings->addNewItem();
	this->syncSettings();
}
void GPC::removeCustomSettingRow(std::string const& name){
	this->settings->removeItem(name);
	this->syncSettings();
	this->modificationsUnsaved = true;
}

void GPC::updateShowMenuSetting(){
	std::string val = this->settings->getValue("GRUB_HIDDEN_TIMEOUT");
	if (val == "" || val.find_first_not_of("0123456789") != -1) {
		this->settings->setValue("GRUB_HIDDEN_TIMEOUT", "0");
	}
	this->settings->setIsActive("GRUB_HIDDEN_TIMEOUT", !this->settingsDlg->getShowMenuCheckboxState());
	if (!this->settingsDlg->getShowMenuCheckboxState() && this->settingsDlg->getOsProberCheckboxState()){
		this->settingsDlg->showHiddenMenuOsProberConflictMessage();
	}
	this->syncSettings();
	this->modificationsUnsaved = true;
}

void GPC::updateOsProberSetting(){
	this->settings->setValue("GRUB_DISABLE_OS_PROBER", this->settingsDlg->getOsProberCheckboxState() ? "false" : "true");
	this->settings->setIsActive("GRUB_DISABLE_OS_PROBER", !this->settingsDlg->getOsProberCheckboxState());
	this->syncSettings();
	this->modificationsUnsaved = true;
}

void GPC::updateKernalParams(){
	this->settings->setValue("GRUB_CMDLINE_LINUX_DEFAULT", this->settingsDlg->getKernelParams());
	this->syncSettings();
	this->modificationsUnsaved = true;
}

void GPC::updateUseCustomResolution(){
	if (this->settings->getValue("GRUB_GFXMODE") == "") {
		this->settings->setValue("GRUB_GFXMODE", "saved"); 
	}
	this->settings->setIsActive("GRUB_GFXMODE", this->settingsDlg->getResolutionCheckboxState());
	this->syncSettings();
	this->modificationsUnsaved = true;
}

void GPC::updateBackgroundImage(){
	this->settings->setValue("GRUB_MENU_PICTURE", this->settingsDlg->getBackgroundImagePath());
	this->settings->setIsActive("GRUB_MENU_PICTURE", true);
	this->settings->setIsExport("GRUB_MENU_PICTURE", true);
	this->syncSettings();
	this->modificationsUnsaved = true;
}

void GPC::updateColorSettings(){
	if (this->settingsDlg->getColorChooser(SettingsDlg::COLOR_CHOOSER_DEFAULT_FONT).getSelectedColor() != "" && this->settingsDlg->getColorChooser(SettingsDlg::COLOR_CHOOSER_DEFAULT_BACKGROUND).getSelectedColor() != ""){
		this->settings->setValue("GRUB_COLOR_NORMAL", this->settingsDlg->getColorChooser(SettingsDlg::COLOR_CHOOSER_DEFAULT_FONT).getSelectedColor() + "/" + this->settingsDlg->getColorChooser(SettingsDlg::COLOR_CHOOSER_DEFAULT_BACKGROUND).getSelectedColor());
		this->settings->setIsActive("GRUB_COLOR_NORMAL", true);
		this->settings->setIsExport("GRUB_COLOR_NORMAL", true);
	}
	if (this->settingsDlg->getColorChooser(SettingsDlg::COLOR_CHOOSER_HIGHLIGHT_FONT).getSelectedColor() != "" && this->settingsDlg->getColorChooser(SettingsDlg::COLOR_CHOOSER_HIGHLIGHT_BACKGROUND).getSelectedColor() != ""){
		this->settings->setValue("GRUB_COLOR_HIGHLIGHT", this->settingsDlg->getColorChooser(SettingsDlg::COLOR_CHOOSER_HIGHLIGHT_FONT).getSelectedColor() + "/" + this->settingsDlg->getColorChooser(SettingsDlg::COLOR_CHOOSER_HIGHLIGHT_BACKGROUND).getSelectedColor());
		this->settings->setIsActive("GRUB_COLOR_HIGHLIGHT", true);
		this->settings->setIsExport("GRUB_COLOR_HIGHLIGHT", true);
	}
	this->syncSettings();
	this->modificationsUnsaved = true;
}

void GPC::removeBackgroundImage(){
	this->settings->setIsActive("GRUB_MENU_PICTURE", false);
	this->syncSettings();
	this->modificationsUnsaved = true;
}

void GPC::hideSettingsDialog(){
	this->settingsDlg->hide();
	if (this->settings->reloadRequired()){
		this->getThreadController().startLoadThread(true);
	}
}

void GPC::updateTimeoutSetting(){
	if (this->settingsDlg->getShowMenuCheckboxState()){
		this->settings->setValue("GRUB_TIMEOUT", this->settingsDlg->getTimeoutValueString());
	}
	else {
		this->settings->setValue("GRUB_HIDDEN_TIMEOUT", this->settingsDlg->getTimeoutValueString());
	}
	this->syncSettings();
	this->modificationsUnsaved = true;
}

void GPC::updateCustomResolution(){
	this->settings->setValue("GRUB_GFXMODE", this->settingsDlg->getResolution());
	this->syncSettings();
	this->modificationsUnsaved = true;
}

void GPC::updateGenerateRecoverySetting(){
	if (this->settings->getValue("GRUB_DISABLE_LINUX_RECOVERY") != "true") {
		this->settings->setValue("GRUB_DISABLE_LINUX_RECOVERY", "true");
	}
	this->settings->setIsActive("GRUB_DISABLE_LINUX_RECOVERY", !this->settingsDlg->getRecoveryCheckboxState());
	this->syncSettings();
	this->modificationsUnsaved = true;
}
