#include <iostream>
#include <string>
#include <dirent.h>
#include <sys/stat.h>

using namespace std;

bool dfs(const string &path, const string &file){
    DIR *dir = opendir(path.c_str());
    if(!dir) return false;

    struct dirent *current;
    while((current = readdir(dir)) != NULL){
        string name = current->d_name;

        if(name == "." || name == "..") continue;
        if(name == file){
            closedir(dir);
            return true;
        }

        string next_path = path + "/" + name;

        struct stat st;
        if(stat(path.c_str(), &st) != -1){
            if(S_ISDIR(st.st_mode)){
                if(dfs(next_path, file));
                return true;
            }
        }
    }
    closedir(dir);
    return false;
}

void handle_search(const string &command){
    if(command.size() <= 2){
        cout << "File name not provided" << endl;
        return;
    }

    string path = ".";
    string file = command;
    size_t pos = command.find_last_of('/');
    if(pos != string::npos) {   // path provided
        path = command.substr(0, pos);
        if(path.empty()) path = ".";
        file = command.substr(pos+1);
    }

    cout << (dfs(path, file) ? "True" : "False") << endl;
}