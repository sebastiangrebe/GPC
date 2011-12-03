#ifndef GRUBLISTCFGDLG_H_
#define GRUBLISTCFGDLG_H_

#include "evt_listCfgDlg.h"
#include <string>

class GrublistCfgDlg {
public:
	enum Exception {
		PROXY_ITER_NOT_FOUND,
		RULE_ITER_NOT_FOUND
	};
	virtual void setEventListener(EventListener_listCfgDlg& eventListener)=0;
	virtual void show()=0;
	virtual void run()=0;
	virtual void close()=0;
	virtual void showBurgSwitcher()=0;
	virtual void hideBurgSwitcher()=0;
	virtual void showPartitionChooserQuestion()=0;
	virtual void hidePartitionChooserQuestion()=0;
	virtual bool isVisible()=0;
	virtual void setIsBurgMode(bool isBurgMode)=0;
	virtual void setLockState(int state)=0;
	virtual void setProgress(double progress)=0;
	virtual void progress_pulse()=0;
	virtual void hideProgressBar()=0;
	virtual void setStatusText(std::string const& new_status_text)=0;
	virtual void appendScript(std::string const& name, bool is_active, void* proxyPtr)=0;
	virtual void appendEntry(std::string const& name, bool is_active, void* entryPtr, bool editable, bool is_submenu, void* parentEntry = NULL)=0;
	virtual void showProxyNotFoundMessage()=0;
	virtual std::string createNewEntriesPlaceholderString(std::string const& parentMenu = "")=0;
	virtual void setProxyName(void* proxy, std::string const& name, bool isModified)=0;
	virtual void swapProxies(void* a, void* b)=0;
	virtual void setDefaultTitleStatusText(std::string const& str)=0;
	virtual void removeProxy(void* p)=0;
	virtual int showExitConfirmDialog(int type)=0;
	virtual void showErrorMessage(std::string const& msg)=0;
	virtual void clear()=0;
	virtual std::string getRuleName(void* rule)=0;
	virtual void setRuleName(void* rule, std::string const& newName)=0;
	virtual bool getRuleState(void* rule)=0;
	virtual void setRuleState(void* rule, bool newState)=0;
	virtual bool getProxyState(void* proxy)=0;
	virtual void setProxyState(void* proxy, bool isActive)=0;
	virtual void selectRule(void* rule)=0;
};

#endif
