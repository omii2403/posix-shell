#include <iostream>
#include <string>
#include <unistd.h>
#include <limits.h>
using namespace std;

static string prevDir = "";

void handle_cd(const string &s){
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    string currDir = cwd;

    if(s.empty() || s == "~"){
        // If empty string, return to home
        char *home = getenv("HOME");
        if(home){
            prevDir = currDir;
            if(chdir(home) != 0) perror("cd");
        }
    }
    else if(s == ".") prevDir = currDir;
    else if(s == ".."){
        prevDir = currDir;
        if(chdir("..") != 0) perror("cd");
    }
    else if(s == "-"){
        if(prevDir.empty()){
            cerr << "cd: Old Directory is not set" << endl;
        }
        else{
            string temp = prevDir;
            prevDir = currDir;
            if(chdir(temp.c_str()) != 0) perror("cd");
            else cout << temp << endl;
        }
    }
    // Normal Path
    else{
        prevDir = currDir;
        if(chdir(s.c_str()) != 0){
            perror("cd");
        }
    }
}