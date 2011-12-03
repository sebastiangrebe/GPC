
#ifndef SETTINGSDLG_H_
#define SETTINGSDLG_H_

#include "colorChooser.h"
#include "evt_settings.h"

class SettingsDlg {
public:
	enum DefEntryType {
		DEF_ENTRY_PREDEFINED,
		DEF_ENTRY_SAVED
	};
	enum ColorChooserType {
		COLOR_CHOOSER_DEFAULT_FONT,
		COLOR_CHOOSER_DEFAULT_BACKGROUND,
		COLOR_CHOOSER_HIGHLIGHT_FONT,
		COLOR_CHOOSER_HIGHLIGHT_BACKGROUND
	};
	enum Exception {
		REQUESTED_CUSTOM_OPTION_NOT_FOUND
	};
	struct CustomOption {
		std::string name, old_name, value;
		bool isActive;
	};
	virtual void setEventListener(EventListener_settings& eventListener)=0;
	virtual void show(bool burgMode)=0;
	virtual void hide()=0;
	virtual ColorChooser& getColorChooser(ColorChooserType type)=0;
	virtual std::string getSelectedCustomOption()=0;
	virtual void addEntryToDefaultEntryChooser(std::string const& labelPathValue, std::string const& labelPathLabel, std::string const& numericPathValue, std::string const& numericPathLabel)=0;
	virtual void clearDefaultEntryChooser()=0;
	virtual void clearResolutionChooser()=0;
	virtual void addResolution(std::string const& resolution)=0;
	virtual std::string getSelectedDefaultGrubValue()=0;
	virtual void addCustomOption(bool isActive, std::string const& name, std::string const& value)=0;
	virtual void selectCustomOption(std::string const& name)=0;
	virtual void removeAllSettingRows()=0;
	virtual CustomOption getCustomOption(std::string const& name)=0;
	virtual void setActiveDefEntryOption(DefEntryType option)=0;
	virtual DefEntryType getActiveDefEntryOption()=0;
	virtual void setDefEntry(std::string const& defEntry)=0;
	virtual void setShowMenuCheckboxState(bool isActive)=0;
	virtual bool getShowMenuCheckboxState()=0;
	virtual void setOsProberCheckboxState(bool isActive)=0;
	virtual bool getOsProberCheckboxState()=0;
	virtual void showHiddenMenuOsProberConflictMessage()=0;
	virtual void setTimeoutValue(int value)=0;
	virtual int getTimeoutValue()=0;
	virtual std::string getTimeoutValueString()=0;
	virtual void setKernelParams(std::string const& params)=0;
	virtual std::string getKernelParams()=0;
	virtual void setRecoveryCheckboxState(bool isActive)=0;
	virtual bool getRecoveryCheckboxState()=0;
	virtual void setResolutionCheckboxState(bool isActive)=0;
	virtual bool getResolutionCheckboxState()=0;
	virtual void setResolution(std::string const& resolution)=0;
	virtual std::string getResolution()=0;
	virtual void setBackgroundImagePreviewPath(std::string const& menuPicturePath, bool isInGrubDir)=0;
	virtual std::string getBackgroundImagePath()=0;
};

#endif
