#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>
#include "commands.h"

using namespace std;

extern pid_t fg_pid; // foreground process id

// clean the string by removing extra chars from begining and end
string clean(const string &s) {
    string t = s;
    while(!t.empty() && (t.back() == '\n' || t.back() == '\r' || t.back() == ' ')) t.pop_back();
    while(!t.empty() && (t.front() == ' ' || t.front() == '\t')) t.erase(t.begin());
    return t;
}

// convert string to vector<vector<string>> according to pipe and spaces
vector<vector<string>> build_pipeline(const string &s){
    vector<vector<string>> stages;
    vector<string> current;
    string t = "";
    bool flag1 = false, flag2 = false, flag3 = false;

    for(int i= 0; i< s.size();i++){
        if(s[i] == '\n') continue;
        // detect pipeline/redirection outside quotes
        if(!flag1 && !flag2 && (s[i] == '|')){
            if(!t.empty()){
                // clear qutoes before adding
                t = strip_quotes(t);
                current.push_back(t);
                t = "";
            }
            if(!current.empty()){
                stages.push_back(current);
                current.clear();
            }
            continue;
        }
        if(!flag1 && !flag2 && (s[i] == '>' || s[i] == '<')){
            if(!t.empty()){
                t = strip_quotes(t);
                current.push_back(t);
                t = "";
            }
            if(s[i] == '>' && i+1 < s.size() && s[i + 1] == '>'){
                current.push_back(">>");
                i++;
            }
            else{
                string temp = "";
                temp += s[i];
                temp = strip_quotes(temp);
                current.push_back(temp);
            }
            continue;
        }

        // inside quotee, then keep the chars
        if((flag1 || flag2) && s[i] != '\'' && s[i] != '"'){
            t += s[i];
            continue;
        }

        // toggle single quote
        if(s[i] == '\''){
            flag1 = !flag1;
            if (flag1 == false) flag3 = false;
            t += s[i];
            continue;
        }

        // toggle double quote
        if(s[i] == '"'){
            flag2 = !flag2;
            if (flag2 == false) flag3 = false;
            t += s[i];
            continue;
        }

        // outside quotes: space handling
        if(!flag3 && s[i] == ' '){
            if (!t.empty()) {
                t = strip_quotes(t);
                current.push_back(t);
                t = "";
            }
            flag3 = true;
            continue;
        }

        // normal character
        if(s[i] != ' '){
            t += s[i];
            flag3 = false;
            continue;
        }
    }

    if (!t.empty()){
        t = strip_quotes(t);
        current.push_back(t);
    }
    if (!current.empty()) stages.push_back(current);

    return stages;
}


// for debugging purpose, print all the stages
void printStages(const vector<vector<string>>& stages) {
    for (size_t i = 0; i < stages.size(); ++i) {
        cout << "Stage " << i << ": ";
        for (const string& token : stages[i]) {
            cout << token << ", ";
        }
        cout << endl;
    }
}

void handle_pipe(const string &command){
    vector<vector<string>> stages = build_pipeline(command);
    int total = stages.size();
    if(total == 0) return;

    // Allocate all pipes by using array of file desciptors.
    vector<int> v(2*(total - 1));
    for(int i=0;i < total - 1;i++){
        if(pipe(&v[2*i]) < 0){ // In the gap of 2 each time
            perror("pipe");
            return;
        }
    }

    // process each stage by stagee
    for(int i=0;i < total; i++){
        pid_t pid = fork();
        if(pid < 0){
            perror("fork");
            return;
        }
        else if(!pid){ // children process

            // connect stdin of the previous pipe if it is not the first pipe
            if(i > 0){
                if(dup2(v[(i-1) *2], STDIN_FILENO) < 0){
                    perror("dup2 input");
                    exit(1);
                }
            }

            // connect stdout to next pipe if it is not the last pipe
            if(i < total - 1){
                if(dup2(v[i*2 + 1], STDOUT_FILENO) < 0){
                    perror("dup2 output");
                    exit(1);
                }
            }

            // close all pipe file descriptors in child
            for(auto it: v) close(it);

            // handle parsing of command args and redirection
            string ifile, ofile;
            bool append = false;
            vector<string> keep;   // keep tokens alive
            vector<char *> argv;   // execvp argv

            for(size_t j=0; j < stages[i].size(); j++){
                string t = clean(stages[i][j]);
                if(t == "<" && j+1 < stages[i].size()) ifile = stages[i][++j];
                else if(t == ">" && j+1 < stages[i].size()){
                    ofile = stages[i][++j];
                    append = false;
                }
                else if(t == ">>" && j+1 < stages[i].size()){
                    ofile = stages[i][++j];
                    append = true;
                }
                else {
                    keep.push_back(t);
                }
            }
            
            // prepare arguments array for execvp
            for(auto &s : keep) argv.push_back(const_cast<char*>(s.c_str()));
            argv.push_back(nullptr);

            // handle input redirection
            if(!ifile.empty()){
                int fd = open(ifile.c_str(), O_RDONLY);
                if(fd < 0){
                    perror("open input");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            // handle output redirection
            if(!ofile.empty()){
                int fd;
                // Create a new file if not present
                if(append) fd = open(ofile.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
                else fd = open(ofile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if(fd < 0){
                    perror("open output");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            // execute
            setpgid(0, 0); // put child in new process group
            if(execvp(argv[0], argv.data()) < 0) {
                perror("execvp");
                exit(1);
            }
        }
        
        else{
            if(i == total - 1) fg_pid = pid;
        }
    }

    // parent process closes all pipe file descriptors
    for(int fd: v) close(fd);

    // wait for the whole pipeline process group
    for(int i=0;i<total ; i++){
        int s;
        wait(&s);
    }
    fg_pid = -1;
}
