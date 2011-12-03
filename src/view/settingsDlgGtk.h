#ifndef SETTING_DLG_GTK_INCLUDED
#define SETTING_DLG_GTK_INCLUDED
#include <gtkmm.h>
#include "../interface/settingsDlg.h"
#include <libintl.h>
#include "../model/grubEnv.h"
#include "../interface/colorChooser.h"
#include "../interface/evt_settings.h"
#include <string>
#include "../presenter/commonClass.h"

class ColorChooserGtk : public Gtk::ComboBox, public ColorChooser {
	struct Columns : public Gtk::TreeModelColumnRecord {
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<Glib::ustring> idName;
		Gtk::TreeModelColumn<Glib::ustring> colorCode_background;
		Gtk::TreeModelColumn<Glib::ustring> colorCode_foreground;
		Columns();
	};
	Columns columns;
	Glib::RefPtr<Gtk::ListStore> refListStore;
	public:
	ColorChooserGtk();
	void addColor(std::string const& codeName, std::string const& outputName, std::string const& cell_background, std::string const& cell_foreground);
	void selectColor(std::string const& codeName);
	std::string getSelectedColor() const;
};

class GrubColorChooser : public ColorChooserGtk {
public:
	GrubColorChooser(bool blackIsTransparent = false);
};

class GrubSettingsDlgGtk : public Gtk::Dialog, public SettingsDlg, public CommonClass {
	struct AdvancedSettingsTreeModel : public Gtk::TreeModelColumnRecord {
		Gtk::TreeModelColumn<bool> active;
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<Glib::ustring> old_name;
		Gtk::TreeModelColumn<Glib::ustring> value;
		AdvancedSettingsTreeModel();
	};
	struct CustomOption_obj : public CustomOption {
		CustomOption_obj(std::string name, std::string old_name, std::string value, bool isActive);
	};
	AdvancedSettingsTreeModel asTreeModel;
	Glib::RefPtr<Gtk::ListStore> refAsListStore;
	bool event_lock;
	EventListener_settings* eventListener;
	
	Gtk::Notebook tabbox;
	Gtk::ScrolledWindow scrAllEntries;
	Gtk::TreeView tvAllEntries;
	Gtk::VBox vbAllEntries;
	Gtk::HBox hbAllEntriesControl;
	Gtk::Button bttAddCustomEntry, bttRemoveCustomEntry;

	Gtk::VBox vbCommonSettings, vbAppearanceSettings;
	Gtk::Alignment alignCommonSettings;
	
	Pango::AttrList attrDefaultEntry;
	Pango::Attribute aDefaultEntry;
	Gtk::Frame groupDefaultEntry;
	Gtk::Alignment alignDefaultEntry;
	Gtk::Label lblDefaultEntry;
	Gtk::RadioButton rbDefPredefined, rbDefSaved;
	Gtk::RadioButtonGroup rbgDefEntry;
	Gtk::VBox vbDefaultEntry;
	Gtk::HBox hbDefPredefined;
	Gtk::ComboBoxText cbDefEntry;
	std::map<int, std::string> defEntryValueMapping;
	Gtk::Frame groupView;
	Gtk::Alignment alignView;
	Gtk::Label lblView;
	Gtk::VBox vbView;
	Gtk::CheckButton chkShowMenu, chkOsProber;
	Gtk::HBox hbTimeout;
	Gtk::Label lblTimeout;
	Gtk::SpinButton spTimeout;
	Gtk::Label lblTimeout2;
	Gtk::Frame groupKernelParams;
	Gtk::Alignment alignKernelParams;
	Gtk::Label lblKernelParams;
	Gtk::VBox vbKernelParams;
	Gtk::Entry txtKernelParams;
	Gtk::CheckButton chkGenerateRecovery;
	Gtk::Alignment alignResolution;
	Gtk::HBox hbResolution;
	Gtk::CheckButton chkResolution;
	Gtk::ComboBoxEntryText cbResolution;
	Gtk::Frame groupColorChooser;
	Gtk::Alignment alignColorChooser;
	Gtk::Label lblColorChooser;
	Gtk::Table tblColorChooser;
	Gtk::Label lblforegroundColor, lblBackgroundColor, lblNormalColor, lblHighlightColor;
	GrubColorChooser gccNormalForeground, gccNormalBackground, gccHighlightForeground, gccHighlightBackground;
	Gtk::Frame groupBackgroundImage;
	Gtk::Alignment alignBackgroundImage;
	Gtk::Label lblBackgroundImage, lblBackgroundRequiredInfo;
	Gtk::VBox vbBackgroundImage;
	Gtk::FileChooserButton fcBackgroundImage;
	Gtk::Image imgBackgroundImage;
	Gtk::HBox hbImgBtts;
	Gtk::VBox vbButtons;
	Gtk::Button bttCopyBackground, bttRemoveBackground;
	
	void signal_setting_row_changed(const Gtk::TreeModel::Path& path, const Gtk::TreeModel::iterator& iter);
	void signal_add_row_button_clicked();
	void signal_remove_row_button_clicked();
	void signal_default_entry_predefined_toggeled();
	void signal_default_entry_saved_toggeled();
	void signal_default_entry_changed();
	void signal_showMenu_toggled();
	void signal_osProber_toggled();
	void signal_timeout_changed();
	void signal_kernelparams_changed();
	void signal_recovery_toggled();
	void signal_chkResolution_toggled();
	void signal_resolution_selected();
	void signal_color_changed();
	void signal_other_image_chosen();
	void signal_bttRemoveBackground_clicked();
	void on_response(int response_id);
	public:
	GrubSettingsDlgGtk();
	void setEventListener(EventListener_settings& eventListener);
	void show(bool burgMode);
	void hide();
	ColorChooser& getColorChooser(ColorChooserType type);
	void addEntryToDefaultEntryChooser(std::string const& labelPathValue, std::string const& labelPathLabel, std::string const& numericPathValue, std::string const& numericPathLabel);
	void clearDefaultEntryChooser();
	void clearResolutionChooser();
	void addResolution(std::string const& resolution);
	std::string getSelectedDefaultGrubValue();
	void addCustomOption(bool isActive, std::string const& name, std::string const& value);
	void selectCustomOption(std::string const& name);
	std::string getSelectedCustomOption();
	void removeAllSettingRows();
	CustomOption getCustomOption(std::string const& name);
	void setActiveDefEntryOption(DefEntryType option);
	DefEntryType getActiveDefEntryOption();
	void setDefEntry(std::string const& defEntry);
	void setShowMenuCheckboxState(bool isActive);
	bool getShowMenuCheckboxState();
	void setOsProberCheckboxState(bool isActive);
	bool getOsProberCheckboxState();
	void showHiddenMenuOsProberConflictMessage();
	void setTimeoutValue(int value);
	int getTimeoutValue();
	std::string getTimeoutValueString();
	void setKernelParams(std::string const& params);
	std::string getKernelParams();
	void setRecoveryCheckboxState(bool isActive);
	bool getRecoveryCheckboxState();
	void setResolutionCheckboxState(bool isActive);
	bool getResolutionCheckboxState();
	void setResolution(std::string const& resolution);
	std::string getResolution();
	void setBackgroundImagePreviewPath(std::string const& menuPicturePath, bool isInGrubDir);
	std::string getBackgroundImagePath();
};

#endif
