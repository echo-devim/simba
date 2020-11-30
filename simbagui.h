#ifndef __SIMBAGUI_H__
#define __SIMBAGUI_H__

#include "simba.h"
#include <gtkmm.h>
#include <gtkmm/application.h>
#include <gtkmm/messagedialog.h>

using namespace std;

class SimbaGUI {
private:
    Simba *simba;
    Glib::RefPtr<Gtk::Application> app;
    void showUI();
    void on_item_click(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
public:
    SimbaGUI(string &server_ip, string &share, string &domain, string &username);
    ~SimbaGUI();
};

#endif