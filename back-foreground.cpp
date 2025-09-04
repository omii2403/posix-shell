#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;

// get external global variables from main.cpp
extern pid_t fg_pid;
extern vector<pid_t> bg_pid;

void handle_system(const string& command) {
    string args = command;
    if(args.size() == 0) return;

    bool bg= false;
    if(args[args.size() - 1] == '&'){
        // if & is found then the process is background, so set the flag true
        bg = true;
        args.pop_back();
        if(args.size() > 0 && args[args.size() - 1] == ' ') args.pop_back();
    }

    // create tokens separated by space
    vector<string> tokens;
    string token = "";
    for(int i = 0; i < args.size(); i++){
        if(args[i] == ' '){
            if(!token.empty()){
                tokens.push_back(token);
                token = "";
            }
        }
        else token += args[i];
    }
    if(!token.empty()) tokens.push_back(token);

    // if no tokens founded then return
    if(tokens.empty()) return;

    // Convert the arguments to char* array
    vector<char*> argv;
    for(auto &t : tokens) argv.push_back(const_cast<char*>(t.c_str()));
    argv.push_back(nullptr);

    // fork and execute the command
    pid_t pid;
    if((pid = fork()) < 0){
        perror("fork");
        return;
    }
    if(!pid){ // child process if pid == 0
        setpgid(0, 0); // group processes
        if (execvp(argv[0], argv.data()) == -1) {
            perror("Command not found");
            exit(EXIT_FAILURE);
        }
    }
    else{
        // Parent process
        if(!bg){
            // for foreground processss
            fg_pid = pid;  // track PGID = pid
            int status;
            waitpid(pid, &status, WUNTRACED); // wait also for stop (Ctrl-Z)
            fg_pid = -1;
        }
        else{
            // this is for background process
            cout << "Process started in background with PID " << pid << endl;
            bg_pid.push_back(pid);
        }
    }
}