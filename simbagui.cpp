#include "simbagui.h"

SimbaGUI::SimbaGUI(string &server_ip, string &share, string &domain, string &username) {
    this->simba = nullptr;
    this->current_path = "";
    this->app = Gtk::Application::create("org.gtkmm.simba");
    Glib::ustring message = "Password:";
    Gtk::MessageDialog *inputbox = new Gtk::MessageDialog(message);
    inputbox->set_title("Auth");
    Gtk::Box *content = inputbox->get_content_area();
    Gtk::Entry *entry = new Gtk::Entry();
    entry->set_visibility(false);
    entry->set_invisible_char('*');
    entry->set_size_request(40,10);
    content->pack_end(*(dynamic_cast<Gtk::Widget*>(entry)), false, false, 0);
    inputbox->show_all();
    int response = inputbox->run();
    Glib::ustring raw_pass = entry->get_text();
    delete inputbox;
    string pass = raw_pass;
    if ((response == Gtk::RESPONSE_OK) && (pass != "")) {
        this->simba = new Simba(server_ip, share, domain, username, pass);
        if (this->simba->isLogged()) {
            this->showUI();
        } else {
            Gtk::MessageDialog errorbox = Gtk::MessageDialog("Auth failed");
            errorbox.set_title("Error");
            errorbox.show_all();
            errorbox.run();
        }
    }
}

void SimbaGUI::on_item_click(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column) {
    auto model = tree_view->get_model();
    auto iter = model->get_iter(path);
    Gtk::TreeModel::Row row = *iter;
    if (current_path != "")
        current_path += "\\";
    current_path += row.get_value(itemval);
    this->pathbar->set_text(current_path);
    updateTreeView();
}

void SimbaGUI::updateTreeView() {
    list_store->clear();
    vector<string> files;
    if (this->current_path == "") {
        files = this->simba->getShares();
    } else {
        files = this->simba->list(current_path);
    }
    for (string f : files) {
        if ((f != ".") && (f != "..")) {
            Gtk::TreeModel::Row row = *(list_store->append());
            row[itemval] = f.c_str();
        }
    }
}

bool SimbaGUI::on_pathbar_key_press(GdkEventKey* event) {
    if ((event->type == GDK_KEY_PRESS) && (event->keyval == GDK_KEY_Return)) {
        this->current_path = this->pathbar->get_text();
        updateTreeView();
    }
    return false;
}

void SimbaGUI::showUI() {
    Gtk::Window window;
    window.set_default_size(400, 600);
    window.set_title("Simba");
    Gtk::TreeModel::ColumnRecord colrec;
    colrec.add(itemval);
    list_store = Gtk::ListStore::create(colrec);
    tree_view = new Gtk::TreeView();
    tree_view->set_model(list_store);
    tree_view->append_column("FileName", itemval);
    tree_view->set_activate_on_single_click(false);
    for (string share : this->simba->getShares()) {
        Gtk::TreeModel::Row row = *(list_store->append());
        row[itemval] = share.c_str();
    }
    tree_view->signal_row_activated().connect(sigc::mem_fun(*this, &SimbaGUI::on_item_click));
    Gtk::VBox *vbox = new Gtk::VBox();
    pathbar = new Gtk::Entry();
    dynamic_cast<Gtk::Widget*>(pathbar)->set_size_request(-1, 10);
    dynamic_cast<Gtk::Widget*>(pathbar)->signal_key_press_event().connect(sigc::mem_fun(*this, &SimbaGUI::on_pathbar_key_press), false);
    vbox->pack_start(*(dynamic_cast<Gtk::Widget*>(pathbar)), 0, 0);
    vbox->add(*(dynamic_cast<Gtk::Widget*>(tree_view)));
    window.add(*(dynamic_cast<Gtk::Widget*>(vbox)));
    window.show_all();
    app->run(window);
}

SimbaGUI::~SimbaGUI() {
    if (this->simba != nullptr)
        delete this->simba;
}