#include <iostream>
#include <vector>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <shadow.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

using namespace std;

struct UserInfo {
    string username;
    uid_t uid;
    string home_dir;
    string password_hash;
    vector<pair<string, bool>> groups;
};

bool get_shadow_info(const string& username, string& hash) {

    struct spwd *sp = getspnam(username.c_str());
    if (sp) {
        hash = sp->sp_pwdp;
    }

    return sp != nullptr;
}

vector<UserInfo> get_users_info(bool try_shadow) {
    vector<UserInfo> users;

    setpwent();
    struct passwd *pwd;
    while ((pwd = getpwent()) != nullptr) {
        UserInfo user;
        user.username = pwd->pw_name;
        user.uid = pwd->pw_uid;
        user.home_dir = pwd->pw_dir;

        if (try_shadow) {
            if (!get_shadow_info(pwd->pw_name, user.password_hash)) {
                user.password_hash = "? (access denied)";
            }
        } else {
            user.password_hash = "? (use sudo to see)";
        }

        int ngroups = 0;
        getgrouplist(pwd->pw_name, pwd->pw_gid, nullptr, &ngroups);
        if (ngroups > 0) {
            gid_t groups[ngroups];
            getgrouplist(pwd->pw_name, pwd->pw_gid, groups, &ngroups);

            for (int i = 0; i < ngroups; i++) {
                struct group *grp = getgrgid(groups[i]);
                if (grp) {
                    bool is_admin = (grp->gr_gid == pwd->pw_gid);
                    user.groups.emplace_back(grp->gr_name, is_admin);
                }
            }
        }

        users.push_back(user);
    }
    endpwent();

    setuid(getuid());

    return users;
}

void print_users_info(const vector<UserInfo>& users) {
    for (const auto& user : users) {
        cout << "Username: " << user.username << "\n"
             << "UID: " << user.uid << "\n"
             << "Home: " << user.home_dir << "\n"
             << "Password: " << user.password_hash << "\n"
             << "Groups: ";

        for (const auto& [group, is_admin] : user.groups) {
            cout << group;
            if (is_admin) cout << "*";
            cout << " ";
        }
        cout << "\n----------------------------------------\n";
    }
}

int main() {
    bool try_shadow = (geteuid() == 0);
    
    if (!try_shadow) {
        cout << "Note: Running with normal privileges. Password hashes will be hidden.\n"
             << "To see all information, run with sudo or set SUID bit (not recommended).\n\n";
    }

    auto users = get_users_info(try_shadow);
    print_users_info(users);

    return 0;
}