#include <iostream>
#include <string>
#include <unistd.h>
#include <limits.h>
#include <vector>
#include <cstring>
#include "commands.h"
using namespace std;

static string prevDir = "";

void handle_cd(const string &cmd){
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    string currDir = cwd;

    // tokenize the string
    vector<string> tokens = tokenize(cmd);

    if(tokens.size() >= 2){
        cout << "cd: too many arguments" << endl;
        return;
    }

    string s = strip_quotes(cmd);

    if(s.empty() || s == "~"){
        // If empty string, return to home
        char *home = getenv("HOME");
        if(home){
            prevDir = currDir;
            if(chdir(home) != 0) perror("cd");
        }
    }
    // if . then stay in the same directory
    else if(s == ".") prevDir = currDir;
    else if(s == ".."){
        // move to back directory
        prevDir = currDir;
        if(chdir("..") != 0) perror("cd");
    }
    else if(s == "-"){
        // go back to previous dir
        if(prevDir.empty()) cerr << "cd: Old Directory is not set" << endl;
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