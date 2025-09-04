#include <iostream>
#include <pwd.h> // for getpwuid(), getcwd() function
#include <unistd.h> // for getuid(), gethostname() functions
#include <limits.h> // For PATH_MAX env variable
#include <string>
#include <algorithm> // For remove() function.
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <vector>
#include <wait.h>
#include "commands.h"
using namespace std;

bool finish = false; // global flag to exit from the terminal
bool redirection = false; // to check if the command is of pure redirection
bool pipeline = false; // to check if the command is pipeline
bool bg_flag = false; // to check if the command is for background processes.

pid_t fg_pid = -1; // foreground process id initialized to -1
vector<pid_t> bg_pid; // vector to store background processes, so that they can be handled if they become zombie

// string_processor which removes extra spaces, newline characters, tab chars from the input
string string_processor(string s){
    string t = "";
    // flag1 is for single quotes, flag2 is for double quotes, flag3 is for extra spaces
    bool flag1 = false, flag2 = false, flag3 = false;
    int start = 0, end = s.size() - 1;
    for(int i=0;i<s.size() ;i++){
        // if extra characters found at the beginning of input, skip them
        if(s[i] == ' ' || s[i] == '\n' || s[i] == '\t') continue;
        else{
            // break when no extra chars found
            start = i;
            break;
        }
    }
    for(int i=s.size() - 1;i>=0 ;i--){
        // if extra characters found at the end of the input, skip them
        if(s[i] == ' ' || s[i] == '\n' || s[i] == '\t') continue;
        else{
            // break when no extra chars found
            end = i;
            break;
        }
    }
    for(int i=start;i<= end; i++){
        // if '&' is found outside any quotes, set the background process flag as true
        if(!flag1 && !flag2 && s[i] == '&') bg_flag = true;
        // if '|' is found outside any quotes, set the pipeline flag as true
        if(!flag1 && !flag2 && (s[i] == '|')) pipeline = true;

        // if '>' or '<' is found outside any quotes, set the redirection flag as true
        if(!flag1 && !flag2 && (s[i] == '>' || s[i] == '<')) redirection = true;
        if((flag1 || flag2) && s[i] != '\'' && s[i] != '"'){
            t += s[i];
            continue;
        }
        if(s[i]== '\''){
            flag1 = !flag1;
            if(flag1 == false) flag3 = false;
            t += s[i];
            continue;
        }
        if(s[i] == '"'){
            flag2 = !flag2;
            if(flag2 == false) flag3 = false;
            t += s[i];
            continue;
        }
        
        if(!flag3 && s[i] == ' '){
            t += s[i];
            flag3 = true;
            continue;
        }
        if(s[i] != ' '){
            t += s[i];
            flag3 = false;
            continue;
        }
    }
    return t;
}

// Handle each command
void handle_command(string s){
    try {
        if(s.empty()) return;
        int n = s.size();
        string temp = "";
        int k = 0;
        // separate the first command line by space
        for(int i=0; i < n; i++){
            if(s[i] == ' '){
                k = i;
                break;
            }
            else temp += s[i];
        }
        if(bg_flag){ // handle if background process found
            handle_system(s);
            bg_flag = false;
            pipeline = false;
            redirection = false;
            return;
        }
        
        if(pipeline){ // handle if pipeline found
            handle_pipe(s);
            pipeline = false;
            redirection = false;
            return;
        }
        if(redirection){ // handle if redirection found
            handle_redirection(s);
            redirection = false;
            return;
        }
        if(temp == "cd"){
            // Handle cd
            string arg = "";
            if(k != 0) arg = s.substr(k+1);
            handle_cd(arg);
        }
        else if(temp == "echo"){
            // Handle echo
            string t = "";
            if(k == 0){
                cout << endl;
                return;
            }
            for(int i = k+1; i < n;i++) t += s[i];
            t = strip_quotes(t);
            cout << t << endl;
        }
        else if(temp == "pwd"){
            // Handle pwd
            char curr[PATH_MAX];
            getcwd(curr, sizeof(curr));
            string currDir = curr;
            cout << currDir << endl;
        }
        else if(temp == "exit"){
            finish = true;
            return;
        }
        else if(temp == "ls"){
            string arg = "";
            if(k != 0) arg = s.substr(k+1);
            handle_ls(arg);
        }
        else if(temp == "pinfo"){
            string arg = "";
            if(k != 0) arg = s.substr(k+1);
            handle_pinfo(arg);
        }
        else if(temp == "search"){
            string arg = "";
            if(k != 0) arg = s.substr(k+1);
            handle_search(arg);
        }
        else if (temp == "history") {
            string arg = "";
            if (k != 0) arg = s.substr(k + 1);
            handle_history(arg);
        }
        else{
            // perror(temp.c_str());
            handle_system(s);
            // cerr << temp << ": Command does not exist" << endl;
        }
    }
    catch (const std::exception &e) {
        cerr << "Error: " << e.what() << endl;
    }
    catch (...) {
        cerr << "Unknown error occurred while handling command: " << s << endl;
    }
}

void handle_sigtstp(int sig){
    if(fg_pid > 0){
        kill(-fg_pid, SIGTSTP);
        cout << endl << "Process " << fg_pid << " stopped and moved to background" << endl;
    }
    else{
        cout << endl << "No foreground process to stop" << endl;
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }
}

void handle_sigint(int sig){
    if(fg_pid > 0){
        kill(-fg_pid, SIGINT);   // send SIGINT to the whole fg process group
        cout << endl << "Process " << fg_pid << " interrupted" << endl;
        fg_pid = -1;             // reset foreground tracker
    }
    else{
        cout << endl << "No foreground process to interrupt" << endl;
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }
}

void handle_sigchld(int sig){
    pid_t pid;
    int status;
    while((pid=waitpid(-1, &status, WNOHANG))>0){
        auto it=find(bg_pid.begin(), bg_pid.end(), pid);
        if(it!=bg_pid.end()){
            cout << endl << "[" << pid << "] " << " Done" << endl;
            bg_pid.erase(it);
            rl_on_new_line();
            rl_redisplay();
        }
    }
}

int main(){
    // Get User Name
    struct passwd* user = getpwuid(getuid());
    string userName = user ? user->pw_name : "user";

    // Get host (system) name
    char host[1024];
    gethostname(host, sizeof(host));
    string hostName = host;

    // Get the shell's home directory
    char* home = getenv("HOME");
    string homeDir = string(home);

    // Clear the screens
    cout << "\033[2J\033[H";  // ANSI escape code

    signal(SIGTSTP, handle_sigtstp); // CTRL + Z
    signal(SIGINT, handle_sigint); // CTRL + C
    signal(SIGCHLD, handle_sigchld);

    // TAB
    autocomplete();
    rl_catch_signals = 0;
    // this will show multiple matches
    rl_bind_key('\t', rl_complete);
    rl_completion_query_items =100;
    rl_variable_bind("show-all-if-ambiguous", "on");
    // Bind arrows to history
    rl_bind_keyseq("\\e[A", rl_get_previous_history);
    rl_bind_keyseq("\\e[B", rl_get_next_history);
    // Load the previous history if any
    load_history();
    stifle_history(20); // keep only last 20 commands

    while(1){
        char curr[PATH_MAX];
        getcwd(curr, sizeof(curr));
        string currDir = curr;
        
        // setting up the directory
        if(currDir.find(homeDir) == 0){
            if(currDir == homeDir) currDir = "~";
            else{
                if(currDir.find(homeDir+"/")==0) currDir = "~"+currDir.substr(homeDir.size());
            }
        }
        
        string terminal = userName + "@" + hostName + ":" + currDir + "$ ";

        char *input = readline(terminal.c_str());
        if(!input){
            cout << endl;
            break;
        }
        
        string line(input);
        free(input);
        line.erase(remove(line.begin(), line.end(), '\t'), line.end());

        if (!line.empty()) store_history(line.c_str());
        // Process one by one input.
        string command = "";
        for(int i=0; i<line.size(); i++){
            if(line[i] == ';'){
                pipeline = redirection = bg_flag = false;
                string temp = string_processor(command);
                handle_command(temp);
                command = "";
                if(finish) break;
            }
            else{
                command += line[i];
            }
        }
        if(finish) break;
        if(command != ""){
            string temp = string_processor(command);
            handle_command(temp);
        }
        if(finish) break;
    }
    save_history();
}
