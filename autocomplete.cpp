#include <readline/readline.h>
#include <dirent.h>
#include <vector>
#include <string>
using namespace std;

// Initializing available commands vector.
vector<string> commands = {"cd", "echo", "pwd", "exit", "pinfo", "search", "ls", "cat", "history"};

// Generating function for readline present in main function.
char *generator(const char *text, int state){
    static ssize_t idx;
    static vector<string> matches;

    if(state == 0){
        // fresh call to build list of matches.

        matches.clear(); // clear the match array
        string prefix(text); // Take the current text as prefix

        // Search for that in the builtins and push into matches array if present.
        for(auto &cmd: commands){
            if(cmd.empty()) continue;
            if(cmd.find(prefix) == 0) matches.push_back(cmd);
        }

        // Looking for the text as a file name in the current directory.
        DIR *dir = opendir(".");
        if(dir){
            struct dirent* exists;
            while((exists = readdir(dir)) != nullptr){
                string name = exists -> d_name;
                if(name.find(prefix) == 0) matches.push_back(name);
            }
            closedir(dir);
        }
        idx = 0;
    }

    // return next match
    if(idx < matches.size())  return strdup(matches[idx++].c_str());
    return NULL;
}

char **completion(const char *text, int start, int end){
    // by default, readline does filename completion, here I am overriding it to include commands to, for that I have implemented a generating function

    rl_attempted_completion_over = 1; // Disable default filename completion
    return rl_completion_matches(text, generator);
}

void autocomplete(){
    rl_attempted_completion_function = completion;
}