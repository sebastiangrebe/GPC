#include "aboutDialogGtk.h"

void AboutDialogGtk::signal_about_dlg_response(int response_id){
	if (Gtk::RESPONSE_CLOSE)
		this->hide();
}

AboutDialogGtk::AboutDialogGtk()
	: appName("GPC"), appVersion(GC_VERSION)
{
	this->set_name(appName);
	this->set_version(appVersion);
	this->authors.push_back("Sebastian Grebe https://launchpad.net/~sebigrebe");
	this->set_authors(authors);
	this->set_icon_name("gpc");
	this->set_logo_icon_name("gpc");
	this->set_comments(gettext("GPC is a graphical interface to configure grub"));
	this->signal_response().connect(sigc::mem_fun(this, &AboutDialogGtk::signal_about_dlg_response));
}

void AboutDialogGtk::show(){
	Gtk::AboutDialog::show();
}
