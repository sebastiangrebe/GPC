#ifndef SCRIPTADDDLG_H_
#define SCRIPTADDDLG_H_

#include "evt_scriptAddDlg.h"

class ScriptAddDlg {
public:
	virtual void setEventListener(EventListener_scriptAddDlg& eventListener)=0;
	virtual void clear()=0;
	virtual void addItem(std::string const& text)=0;
	virtual int getSelectedEntryIndex()=0;
	virtual void clearPreview()=0;
	virtual void addToPreview(std::string const& name)=0;
	virtual void show()=0;
	virtual void hide()=0;
};

#endif
