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
    Gtk::Label *statusbar;
    Gtk::Entry *pathbar;
    Gtk::TreeView *tree_view;
    Gtk::TreeModelColumn<Glib::ustring> itemval;
    Glib::RefPtr< Gtk::ListStore > list_store;
    Glib::RefPtr<Gtk::Application> app;
    string current_path;
    void showUI();
    void on_item_click(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
    bool on_pathbar_key_press(GdkEventKey* event);
    void updateTreeView();
    void on_btnUpload_click();
    void on_btnDownload_click();
public:
    SimbaGUI(string &server_ip, string &share, string &domain, string &username);
    ~SimbaGUI();
};

#endif