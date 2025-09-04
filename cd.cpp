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
    char buf[cmd.size() + 1];
    strcpy(buf, cmd.c_str());

    vector<char*> tokens;
    char *tok = strtok(buf, " ");
    while (tok != NULL) {
        tokens.push_back(tok);
        tok = strtok(NULL, " ");
    }

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