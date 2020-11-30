#include "cxxopts.h"
#include <termios.h>
#include <unistd.h>

#ifdef ENABLE_GUI
#include "simbagui.h"
#else
#include "simba.h"
#endif
using namespace std;

void printHelp() {
    cout << "Commands:" << endl;
    cout << "ls <path>\tList files and directories" << endl;
    cout << "get <path>\tDonwload the specified file to current directory" << endl;
    cout << "put <local path>,<remote path>\tUpload the specified local file to remote path" << endl;
    cout << "rm <path>\tRemove the specified file" << endl;
    cout << "mv <old path>,<new path>\tMove the specified file from oldpath to newpath" << endl;
    cout << "mkdir <path>\tcreate a new directory" << endl;
    cout << "rmdir <path>\tRemove a directory" << endl;
}

int main(int argc, char *argv[])
{
    cxxopts::Options options("Simba", "Lightweight SMBv1 client");
    options.add_options()
    ("h,help", "Show this help menu")
    #ifdef ENABLE_GUI
    ("g,gui", "Show GTK3 gui")
    #endif
    ("s,server", "SMB Server IP Address", cxxopts::value<std::string>())
    ("d,domain", "Domain/Workgroup", cxxopts::value<std::string>())
    ("a,share", "Share name", cxxopts::value<std::string>())
    ("u,username", "Username", cxxopts::value<std::string>())
    ;
    bool no_args = argc < 4;
    auto result = options.parse(argc, argv);
    if (no_args || (result["help"].count() > 0)) {
        cout << options.help() << endl;
        return 0;
    }
    string server = result["server"].as<string>();
    string domain = result["domain"].as<string>();
    string share = result["share"].as<string>();
    string username = result["username"].as<string>();
    #ifdef ENABLE_GUI
    if (result["gui"].count() > 0) {
        SimbaGUI simbagui(server, share, domain, username);
        return 0;
    }
    #endif
    string password;
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    cout << "Insert password: ";
    cin >> password;
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    Simba simba(server, share, domain, username, password);

    string command;
    while (command != "exit") {
        std::cout << "> ";
        getline(cin, command);
        if ((command == "shares") || (command == "ls")) {
            for (string p : simba.getShares()) {
                std::cout << "\t" << p << endl;
            }
        } else if (command.substr(0,3) == "ls ") {
            size_t pos = command.find(" ");
            if (pos != std::string::npos) {
                string path = command.substr(pos+1);
                std::cout << path << ":" << endl;
                for (string p : simba.list(path)) {
                    if ((p != ".") && (p != ".."))
                        std::cout << "\t" << p << endl;
                }
            } else {
                cerr << "Path required" << endl;
            }
        } else if (command.substr(0, 4) == "get ") {
            size_t pos = command.find(" ");
            if (pos != std::string::npos) {
                string path = command.substr(pos+1);
                if (simba.download(path)) {
                    cout << "Download completed" << endl;
                }
            } else {
                cerr << "Path required" << endl;
            }
        } else if (command.substr(0, 4) == "put ") {
            size_t pos = command.find(" ");
            size_t possep = command.find(",");
            if ((pos != std::string::npos) && (possep != std::string::npos)) {
                string paths = command.substr(pos+1);
                string localpath = paths.substr(0, possep-pos-1);
                string remotepath = paths.substr(possep-pos);
                cout << "Uploading from " << localpath << " to " << remotepath << endl;
                if (simba.upload(localpath, remotepath)) {
                    cout << "Upload completed" << endl;
                }
            } else {
                cerr << "Local path,remote path required" << endl;
            }
        } else if (command.substr(0, 3) == "rm ") {
            size_t pos = command.find(" ");
            if (pos != std::string::npos) {
                string path = command.substr(pos+1);
                simba.remove(path);
            } else {
                cerr << "Path required" << endl;
            }
        } else if (command.substr(0, 3) == "mv ") {
            size_t pos = command.find(" ");
            size_t possep = command.find(",");
            if ((pos != std::string::npos) && (possep != std::string::npos)) {
                string paths = command.substr(pos+1);
                string oldpath = paths.substr(0, possep-pos-1);
                string newpath = paths.substr(possep-pos);
                simba.move(oldpath, newpath);
            } else {
                cerr << "old path,new path required" << endl;
            }
        } else if (command.substr(0, 6) == "mkdir ") {
            size_t pos = command.find(" ");
            if (pos != std::string::npos) {
                string path = command.substr(pos+1);
                simba.makedir(path);
            } else {
                cerr << "Path required" << endl;
            }
        } else if (command.substr(0, 6) == "rmdir ") {
            size_t pos = command.find(" ");
            if (pos != std::string::npos) {
                string path = command.substr(pos+1);
                simba.removedir(path);
            } else {
                cerr << "Path required" << endl;
            }
        } else if (command.substr(0, 4) == "help") {
            printHelp();
        }
    }

    return 0;
}
