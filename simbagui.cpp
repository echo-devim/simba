#include "simbagui.h"

SimbaGUI::SimbaGUI(string &server_ip, string &share, string &domain, string &username) {
    this->simba = nullptr;
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
        this->showUI();
        /*
        if (this->simba->isLogged()) {
            this->showUI();
        } else {
            Gtk::MessageDialog errorbox = Gtk::MessageDialog("Auth failed");
            errorbox.set_title("Error");
            errorbox.show_all();
            errorbox.run();
        }*/
    }
}

void SimbaGUI::on_item_click(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column) {
    cout << "todo" << endl;
}

void SimbaGUI::showUI() {
    Gtk::Window window;
    window.set_default_size(400, 600);
    window.set_title("Simba");
    Gtk::TreeModelColumn<Glib::ustring> filename;
    Gtk::TreeModel::ColumnRecord colrec;
    colrec.add(filename);
    auto list_store = Gtk::ListStore::create(colrec);
    Gtk::TreeView *tree_view = new Gtk::TreeView();
    tree_view->set_model(list_store);
    tree_view->append_column("FileName", filename);
    tree_view->set_activate_on_single_click(false);
    for (string share : this->simba->getShares()) {
        Gtk::TreeModel::Row row = *(list_store->append());
        row[filename] = share.c_str();
    }
    tree_view->signal_row_activated().connect(sigc::mem_fun(*this, &SimbaGUI::on_item_click));
    Gtk::VBox *vbox = new Gtk::VBox();
    vbox->add(*(dynamic_cast<Gtk::Widget*>(tree_view)));
    window.add(*(dynamic_cast<Gtk::Widget*>(vbox)));
    window.show_all();
    app->run(window);
}

SimbaGUI::~SimbaGUI() {
    if (this->simba != nullptr)
        delete this->simba;
}