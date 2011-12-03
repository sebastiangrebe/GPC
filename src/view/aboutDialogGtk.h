#ifndef GC_ABOUNTDIALOG_GTK_INCLUDED
#define GC_ABOUNTDIALOG_GTK_INCLUDED
#include <gtkmm.h>
#include "../config.h"
#include <libintl.h>
#include "../interface/aboutDialog.h"
#include "../presenter/commonClass.h"

class AboutDialogGtk : public Gtk::AboutDialog, public AboutDialog, public CommonClass {
	Glib::ustring appName, appVersion;
	std::vector<Glib::ustring> authors;

	void signal_about_dlg_response(int response_id);
public:
	AboutDialogGtk();
	void show();
};

#endif
