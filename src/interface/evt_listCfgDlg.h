
#ifndef EVENTLISTENER_LISTCFGDLG_INCLUDED
#define EVENTLISTENER_LISTCFGDLG_INCLUDED


class EventListener_listCfgDlg {
public:
	virtual void settings_dialog_request()=0;
	virtual void reload_request()=0;
	virtual void save_request()=0;
	virtual void rootSelector_request()=0;
	virtual void installDialogRequest()=0;
	virtual void scriptAddDlg_requested()=0;
	virtual void removeProxy_requested(void* p)=0;
	virtual void aboutDialog_requested()=0;
	virtual void exitRequest()=0;

	virtual void signal_script_state_toggled(void* script)=0;
	virtual void signal_entry_state_toggled(void* entry)=0;
	virtual void signal_entry_renamed(void* entry)=0;
	virtual void ruleAdjustment_requested(void* rule, int direction)=0;
	virtual void proxySwap_requested(void* a, void* b)=0;
	virtual void ruleSelected(void* rule)=0;
	virtual void proxySelected(void* proxy)=0;
	virtual void burgSwitcher_cancelled()=0;
	virtual void burgSwitcher_response(bool burgChosen)=0;
	virtual void partitionChooserQuestion_response(bool is_positive)=0;
};

#endif
