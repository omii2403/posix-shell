#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include "commands.h"
using namespace std;

void handle_ls(const string &cmd){
    bool flg_a = false, flg_l = false;
    vector<string> dirs; // store multiple directories
    vector<string> args = tokenize(cmd); // tokenize

    for(int i=0;i <args.size();i++){
        string arg = args[i];
        if(arg == "-l") flg_l = true;
        else if(arg =="-a") flg_a = true;
        else if(arg[0] == '-'){
            int j=0;
            while(j < arg.size()){
                if(arg[j] == 'l') flg_l = true;
                else if(arg[j] == 'a') flg_a = true;
                j++;
            }
        }
        else dirs.push_back(arg); // collect directories
    }

    if(dirs.empty()) dirs.push_back("."); // default dir

    for(int d=0; d<dirs.size(); d++){
        string dirPath = dirs[d];
        DIR *dp = opendir(dirPath.c_str());

        if(!dp){
            perror("ls");
            continue;
        }

        if(dirs.size() > 1) cout << dirPath << ":" << endl; // print header if multiple dirs

        vector<string> files;
        struct dirent *de;
        while((de = readdir(dp)) != nullptr){
            string fname = de->d_name;
            if(flg_a || fname[0] != '.') files.push_back(fname);
        }
        closedir(dp);

        // sort in chronolical order
        sort(files.begin(), files.end());

        if(flg_l){
            long long blk = 0;
            vector<struct stat> stv(files.size());

            // collect stats and block count
            for(size_t i=0;i <files.size(); i++){
                string fpath = dirPath + "/" + files[i];
                if(stat(fpath.c_str(), &stv[i]) == -1) continue;
                blk += stv[i].st_blocks;
            }
            // print block count
            cout << "total " << (blk >> 1) << endl;

            // check permissions.
            for(size_t i=0; i<files.size(); i++){
                string fpath = dirPath + "/" + files[i];
                struct stat st;
                if(stat(fpath.c_str(), &st) == -1) continue;

                mode_t m = st.st_mode;
                cout << (S_ISDIR(m) ? 'd' : '-');
                cout << ((m & S_IRUSR) ? 'r' : '-');
                cout << ((m & S_IWUSR) ? 'w' : '-');
                cout << ((m & S_IXUSR) ? 'x' : '-');
                cout << ((m & S_IRGRP) ? 'r' : '-');
                cout << ((m & S_IWGRP) ? 'w' : '-');
                cout << ((m & S_IXGRP) ? 'x' : '-');
                cout << ((m & S_IROTH) ? 'r' : '-');
                cout << ((m & S_IWOTH) ? 'w' : '-');
                cout << ((m & S_IXOTH) ? 'x' : '-');

                cout << ' ' << st.st_nlink << ' ';
                struct group *grp = getgrgid(st.st_gid);
                struct passwd *usr = getpwuid(st.st_uid);
                char tbuf[80];
                struct tm *ti = localtime(&st.st_mtime);
                string uname = (usr ? usr->pw_name : string("?"));
                string gname = (grp? (grp->gr_name): "?");

                cout << uname << ' ' << gname << " " << st.st_size << " ";
                strftime(tbuf, sizeof(tbuf), "%b %d %H:%M", ti);
                cout << tbuf << " ";

                cout << files[i] << endl;
            }
        }
        else{
            for(auto &f : files) cout << f << "  ";
            cout << endl;
        }
        if(dirs.size() > 1 && d < dirs.size()-1) cout << endl; // blank line between dirs
    }
}

