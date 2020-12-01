#include "simba.h"

Simba::Simba(string &server_ip, string &share, string &domain, string &username, string &password) {
    this->logged_in = false;
    struct sockaddr_in addr;
    this->session = smb_session_new();
    if (this->session != NULL) {
        inet_aton(server_ip.c_str(), &addr.sin_addr);
        if (smb_session_connect(session, share.c_str(), addr.sin_addr.s_addr, SMB_TRANSPORT_TCP))
        {
            cerr << "Unable to connect to host" << endl;
        } else {
            cout << "Connected to host" << endl;
            if (this->auth(domain, username, password)) {
                logged_in = true;
                cout << "Logged in";
                if (smb_session_is_guest(session))
                    cout << " as guest";
                cout << endl;
            } else {
                cerr << "Auth failed" << endl;
            }
        }
    } else {
        cerr << "Error, could not create new session" << endl;
    }
}

Simba::~Simba() {
    smb_tree_disconnect(session, tid);
    smb_session_destroy(session);
}

bool Simba::isLogged() {
    return this->logged_in;
}

bool Simba::auth(string &domain, string &username, string &password) {
    smb_session_set_creds(session, domain.c_str(), username.c_str(), password.c_str());
    return (smb_session_login(session) == DSM_SUCCESS);
}

vector<string> Simba::getShares() {
    vector<string> vec;
    char **share_list;
    size_t count = 0;
    if (smb_share_get_list(session, &share_list, &count) != DSM_SUCCESS)
    {
        cerr << "Unable to list shares" << endl;
    } else {
        vec = vector<string>(share_list, share_list + count);
        smb_share_list_destroy(share_list);
    }
    return vec;
}

vector<string> Simba::list(string &path) {
    vector<string> vec;
    string subpath = "";
    string basepath = path;
    size_t pos = path.find("\\");
    if (pos != string::npos) {
        subpath = path.substr(pos+1);
        basepath = path.substr(0, pos);
    }
    smb_tree_disconnect(session, tid);
    int ret = smb_tree_connect(session, basepath.c_str(), &this->tid);
    size_t count = 0;
    if (ret != DSM_SUCCESS)
    {
        cerr << "Unable to list path " << path << endl;
    } else {
        smb_stat_list files = smb_find(session, this->tid, string(subpath + "\\*").c_str());
        size_t files_count = smb_stat_list_count(files);
        if (files_count <= 0)
            cerr << "Unable to list files" << endl;
        else
        {
            for( size_t i = 0; i < files_count; i++ )
            {   
                smb_stat st = smb_stat_list_at( files, i );
                if( st == NULL ) { 
                    cerr << "smb_stat_list_at failed" << endl;
                    break;
                }
                vec.push_back(string(smb_stat_name(st)));
            }   
        }
        smb_stat_list_destroy(files);
    }
    return vec;
}

bool Simba::download_file(smb_file *file, smb_fd fd, string filename) {
    unsigned int chunk = 51200;
    uint64_t len = file->size;
    uint64_t orig_len = len;
    cout << "Downloading " << filename << " (size: " << (static_cast<double>(len)/1024) << " KB)" << endl;
    ofstream out(filename, ios::binary | ios::out);
    while (len > chunk) {
        cout << "\r" << "Progress: " << static_cast<int>(((orig_len - static_cast<double>(len))/orig_len) * 100) << "%";
        char *buffer = new char[chunk];
        smb_fread(session, fd, buffer, chunk);
        out.write(buffer, chunk);
        delete[] buffer;
        len = len - chunk;
    }
    if (len > 0) {
        char *buffer = new char[len];
        smb_fread(session, fd, buffer, len);
        out.write(buffer, len);
        delete[] buffer;
    }
    cout << "\r" << "Progress: 100%" << endl;
    smb_fclose(session, fd);
    out.close();
    return true;
}

bool Simba::isdir(string &path) {
    smb_fd fd;
    if (smb_fopen(session, tid, path.c_str(), SMB_MOD_RO, &fd) != DSM_SUCCESS) {
        cerr << "Unable to open file " << path << endl;
        return false;
    }

    smb_file *file = smb_session_file_get(session, fd);
    bool is_dir = file->is_dir;
    smb_fclose(session, fd);
    return is_dir;
}

bool Simba::download(string &path) {
    smb_fd fd;
    if (smb_fopen(session, tid, path.c_str(), SMB_MOD_RO, &fd) != DSM_SUCCESS) {
        cerr << "Unable to open file " << path << endl;
        return false;
    }

    smb_file *file = smb_session_file_get(session, fd);

    if (file->is_dir) {
        cerr << "Unable to download a directory" << endl;
        smb_fclose(session, fd);
        return false;
    } else {
        return this->download_file(file, fd, path.substr(path.find_last_of("\\")+1));
    }
}

bool Simba::upload_file(smb_fd fd, ifstream &file, string &filename) {
    file.seekg(0, ios::end);
    uint64_t len = file.tellg();
    file.seekg(0, ios::beg);
    unsigned int chunk = 51200;
    uint64_t orig_len = len;
    cout << "Uploading " << filename << " (size: " << (static_cast<double>(len)/1024) << " KB)" << endl;
    while (len > chunk) {
        cout << "\r" << "Progress: " << static_cast<int>(((orig_len - static_cast<double>(len))/orig_len) * 100) << "%";
        char *buffer = new char[chunk];
        file.read(buffer, chunk);
        smb_fwrite(session, fd, buffer, chunk);
        delete[] buffer;
        len = len - chunk;
    }
    if (len > 0) {
        char *buffer = new char[len];
        file.read(buffer, len);
        smb_fwrite(session, fd, buffer, len);
        delete[] buffer;
    }
    cout << "\r" << "Progress: 100%" << endl;
    return true;
}

bool Simba::upload(string &localpath, string &remotepath) {

    ifstream file(localpath, std::ifstream::binary);
    if (!file.good()) {
        cerr << "Unable to open local file " << localpath << endl;
        return false;
    }
    string filename = localpath.substr(localpath.find_last_of("\\")+1);
    if (remotepath[remotepath.length()-1] == '\\') {
        remotepath += filename;
    }
    smb_fd fd;
    if (smb_fopen(session, tid, remotepath.c_str(), SMB_MOD_RW, &fd) != DSM_SUCCESS) {
        cerr << "Unable to open file " << remotepath << endl;
        return false;
    }
    upload_file(fd, file, remotepath);
    smb_fclose(session, fd);
    file.close();
    return true;
}

void Simba::remove(string &path) {
    smb_file_rm(session, tid, path.c_str());
}

void Simba::move(string &oldpath, string &newpath) {
    smb_file_mv(session, tid, oldpath.c_str(), newpath.c_str());
}

void Simba::removedir(string &path) {
    if (smb_directory_rm(session, tid, path.c_str()) == NT_STATUS_DIRECTORY_NOT_EMPTY) {
        cerr << "Directory is not empty" << endl;
    }
}

void Simba::makedir(string &path) {
    smb_directory_create(session, tid, path.c_str());
}