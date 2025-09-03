#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;

extern pid_t foreground_pid;

void handle_system(const string& command) {
    string args = command;
    if(args.size() == 0) return;

    bool bg= false;
    if(args[args.size() - 1] == '&'){
        bg = true;
        args.pop_back();
        if(args.size() > 0 && args[args.size() - 1] == ' ') args.pop_back();
    }

    vector<string> tokens;
    string token = "";
    for (int i = 0; i < args.size(); i++) {
        if (args[i] == ' ') {
            if (!token.empty()) {
                tokens.push_back(token);
                token = "";
            }
        } else {
            token += args[i];
        }
    }
    if (!token.empty()) tokens.push_back(token);

    if (tokens.empty()) return;

    // Convert to char* array for execvp
    vector<char*> argv;
    for (auto &t : tokens) {
        argv.push_back(const_cast<char*>(t.c_str()));
    }
    argv.push_back(nullptr);

    // --- Fork & Exec ---
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        // Child process
        setpgid(0, 0);          // put child in its own group
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        if (execvp(argv[0], argv.data()) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        if (bg) {
            cout << "Process started in background with PID " << pid << endl;
        }
        else {
            foreground_pid = pid;  // track PGID = pid
            int status;
            waitpid(pid, &status, WUNTRACED); // wait also for stop (Ctrl-Z)
            foreground_pid = -1;
            if (WIFSTOPPED(status)) {
                cout << "Process " << pid << " stopped and moved to background" << endl;
            }
        }
    }
}