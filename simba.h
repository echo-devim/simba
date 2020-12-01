#ifndef __SIMBA_H__
#define __SIMBA_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <arpa/inet.h>

extern "C" {
    #include "./libdsm-0.3.2/include/bdsm.h"
    #include "./libdsm-0.3.2/src/smb_fd.h"
}

using namespace std;

class Simba {
private:
    smb_session *session;
    smb_tid tid;
    bool logged_in;
    bool auth(string &domain, string &username, string &password);
    bool download_file(smb_file *file, smb_fd fd, string filename);
    bool upload_file(smb_fd fd, ifstream &file, string &filename);
public:
    Simba(string &server_ip, string &share, string &domain, string &username, string &password);
    ~Simba();
    vector<string> getShares();
    vector<string> list(string &path);
    bool download(string &path);
    bool upload(string &localpath, string &remotepath);
    void remove(string &path);
    void move(string &oldpath, string &newpath);
    void removedir(string &path);
    void makedir(string &path);
    bool isLogged();
    bool isdir(string &path);
};

#endif