#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <iomanip>
#include <sys/types.h>
#include <grp.h>

using namespace std;


struct UserInfo {
    string username;
    string password_hash;
    uid_t uid;
    gid_t gid;
    string home_dir;
    string shell;
};

struct GroupInfo {
    string name;
    gid_t gid;
    vector<string> members;
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

string get_shadow_hash(const string& username) {
    ifstream shadow_file("/etc/shadow");
    string line;

    while (getline(shadow_file, line)) {
        stringstream ss(line);
        string part;
        vector<string> parts;

        while (getline(ss, part, ':')) {
            parts.push_back(part);
        }

        if (parts.size() >= 2 && parts[0] == username) {
            return parts[1];
        }
    }

    return "no_access_or_no_password";
}

unordered_map<gid_t, GroupInfo> read_groups() {
    unordered_map<gid_t, GroupInfo> groups;
    ifstream group_file("/etc/group");
    string line;

    while (getline(group_file, line)) {
        stringstream ss(line);
        string part;
        vector<string> parts;

        while (getline(ss, part, ':')) {
            parts.push_back(part);
        }

        if (parts.size() >= 4) {
            GroupInfo group;
            group.name = parts[0];
            group.gid = stoi(parts[2]);
            
            stringstream members_ss(parts[3]);
            string member;
            while (getline(members_ss, member, ',')) {
                group.members.push_back(member);
            }
            
            groups[group.gid] = group;
        }
    }

    return groups;
}

vector<gid_t> get_user_groups(uid_t uid) {
    vector<gid_t> groups;
    int ngroups = 0;
    
    getgrouplist("", uid, nullptr, &ngroups);
    
    vector<gid_t> group_list(ngroups);
    getgrouplist("", uid, group_list.data(), &ngroups);
    
    return group_list;
}

int main() {
    auto users = read_passwd();
    auto groups_map = read_groups();

    for (const auto& user : users) {
        cout << "Username: " << user.username << "\n";
        cout << "UID: " << user.uid << "\n";
        cout << "GID: " << user.gid << "\n";
        cout << "Home directory: " << user.home_dir << "\n";
        
        string real_hash = get_shadow_hash(user.username);
        cout << "Password hash: " << 
            (user.password_hash == "x" ? real_hash : user.password_hash) << "\n";

        auto user_groups = get_user_groups(user.uid);
        cout << "Groups: ";
        
        for (auto gid : user_groups) {
            if (groups_map.count(gid)) {
                const auto& group = groups_map[gid];
                cout << group.name;
                
                if (!group.members.empty() && group.members[0] == user.username) {
                    cout << "(admin)";
                }
                
                cout << " ";
            }
        }
        
        cout << "\n\n";
    }

    return 0;
}