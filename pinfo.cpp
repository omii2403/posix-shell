#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <string>
using namespace std;

extern pid_t foreground_pid;   // bring in from main.cpp

string fun(const string &path){
    int fd = open(path.c_str(), O_RDONLY);
    if(fd< 0) return "";

    char buf[4096];
    ssize_t n = read(fd, buf, sizeof(buf) -1);
    close(fd);

    if(n <=0) return "";
    buf[n] = '\0';
    return string(buf);
}

void handle_pinfo(const string &arg) {
    pid_t pid;

    if (arg.empty()) pid = getpid();
    else pid = stoi(arg);

    // Get process status
    string statusFile = fun("/proc/" + arg + "/status");
    string state ="?";
    if (!statusFile.empty()) {
        size_t pos = statusFile.find("State:");
        if (pos != string::npos) {
            size_t start = statusFile.find_first_not_of(" \t", pos + 6);
            if (start != string::npos)
                state = statusFile.substr(start, 1); // R/S/Z/T
        }
    }
    // add "+" if this process is foreground
    if (pid == foreground_pid) {
        state += "+";
    }
    

    // Get memory
    string statmContent = fun("/proc/" + arg + "/statm");
    string mem = "?";
    if (!statmContent.empty()) {
        size_t spacePos = statmContent.find(' ');
        if (spacePos != string::npos) mem = statmContent.substr(0, spacePos);
        else mem = statmContent;
    }
    // Get executable path
    string exePath = "/proc/" + arg + "/exe";
    char buf[PATH_MAX];
    ssize_t len = readlink(exePath.c_str(), buf, sizeof(buf) - 1);
    string exe;
    if (len != -1) {
        buf[len] = '\0';
        exe = buf;
    } else {
        exe = "Executable not found";
    }

    // output
    cout << "pid -- " << pid << endl;
    cout << "Process Status -- " << state << endl;
    cout << "memory -- " << mem << " {Virtual Memory}" << endl;
    cout << "Executable Path -- " << exe << endl;
}
