#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <readline/history.h>
using namespace std;

static vector<string> v;
int MAX = 20;
string file;

// Load history from file
void load_history(){
    // Fetching the history file from the home directory.
    const char *home = getenv("HOME");
    file = string(home) + "/.myshell_history";
    int fd = open(file.c_str(), O_RDONLY);

    if (fd < 0) {
        // If file doesn't exist, then create it
        fd = open(file.c_str(), O_WRONLY | O_CREAT, 0644);
        if (fd >= 0) close(fd);  // just create and close
        return; // nothing to load yet
    }

    // Read file content
    string content;
    char buf[1024];
    ssize_t bytes;
    while ((bytes = read(fd, buf, sizeof(buf))) > 0) {
        content.append(buf, bytes);
    }
    close(fd);

    ssize_t start = 0;
    while(start <content.size()){
        ssize_t end = content.find('\n', start);
        // string::npos is the value returned by the find function if it fails.
        // so if not found then assuming it as the end of the file content.
        if(end == string::npos) end = content.size();

        // extracting the next command
        string command= content.substr(start, end-start);
        if(!command.empty()) v.push_back(command); // if found then push it into the command list

        start = end + 1; // move the start pointer to the next one
    }

    // keep only the last MAX commands, erase all the other
    if(v.size() > MAX) v.erase(v.begin(), v.end() - MAX);

    for(const auto &cmd: v) add_history(cmd.c_str());
    stifle_history(MAX);
}

void write_file(const string &path, const vector<string> &history){
    int f = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(f < 0){
        perror("Failed to open history file for writing");
        return;
    }
    // Write the commands separeted by the newline char
    for(const string &cmd: history){
        write(f, cmd.c_str(), cmd.size());
        write(f, "\n", 1);
    }
    close(f);
}

// Save history to file
void save_history(){
    write_file(file, v);
}

// add a new command to history
void store_history(const string &cmd){
    if(cmd.empty()) return; // return if empty command
    v.push_back(cmd);

    if(v.size() > MAX) v.erase(v.begin()); // Erase earliest entry if the size of list exceeds that of MAX
    save_history();
    add_history(cmd.c_str());
    stifle_history(MAX);
}

void handle_history(const string &arg){
    int c = 10; // default to show last 10 commands
    if(!arg.empty()){
        try{
            c = stoi(arg);
        }
        catch(...){
            cout << "Invalid argument" << endl;
            return;
        }
    }

    // Print the last 'c' commands.
    int start = max(0, (int)v.size() - c);

    for(int i=start; i< v.size(); i++) cout << v[i] << endl;
}

