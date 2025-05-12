#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <sys/types.h>
#include <grp.h>
#include <unistd.h>
#include <pwd.h>

using namespace std;

struct UserInfo {
    string username;
    string password_hash;
    uid_t uid;
    gid_t gid;
    string home_dir;
    string shell;
};

vector<UserInfo> read_passwd() {
    vector<UserInfo> users;
    ifstream passwd_file("/etc/passwd");
    string line;

    while (getline(passwd_file, line)) {
        stringstream ss(line);
        string part;
        vector<string> parts;

        while (getline(ss, part, ':')) {
            parts.push_back(part);
        }

        if (parts.size() >= 7) {
            UserInfo user;
            user.username = parts[0];
            user.password_hash = parts[1];
            user.uid = stoi(parts[2]);
            user.gid = stoi(parts[3]);
            user.home_dir = parts[5];
            user.shell = parts[6];
            users.push_back(user);
        }
    }

    return users;
}

vector<string> get_user_groups(const string& username) {
    vector<string> groups;
    gid_t primary_gid = 0;
    
    // Получаем информацию о пользователе
    struct passwd *pw = getpwnam(username.c_str());
    if (!pw) {
        cerr << "User not found: " << username << endl;
        return groups;
    }
    primary_gid = pw->pw_gid;
    
    // Добавляем первичную группу
    struct group *gr = getgrgid(primary_gid);
    if (gr) {
        groups.push_back(gr->gr_name);
    }
    
    // Получаем дополнительные группы
    int ngroups = 0;
    getgrouplist(username.c_str(), primary_gid, nullptr, &ngroups);
    
    if (ngroups > 0) {
        vector<gid_t> gids(ngroups);
        if (getgrouplist(username.c_str(), primary_gid, gids.data(), &ngroups) != -1) {
            for (int i = 0; i < ngroups; i++) {
                if (gids[i] != primary_gid) {  // Уже добавили первичную группу
                    gr = getgrgid(gids[i]);
                    if (gr) {
                        groups.push_back(gr->gr_name);
                    }
                }
            }
        }
    }
    
    return groups;
}

int main() {
    auto users = read_passwd();

    for (const auto& user : users) {
        cout << "Username: " << user.username << "\n";
        cout << "UID: " << user.uid << "\n";
        cout << "GID: " << user.gid << "\n";
        cout << "Home directory: " << user.home_dir << "\n";
        
        auto groups = get_user_groups(user.username);
        cout << "Groups: ";
        bool is_admin = false;
        
        for (const auto& group : groups) {
            cout << group << " ";
            if (group == "sudo" || group == "wheel") {
                is_admin = true;
            }
        }
        
        if (is_admin) {
            cout << "\nStatus: ADMINISTRATOR";
        }
        
        cout << "\n\n";
    }

    return 0;
}