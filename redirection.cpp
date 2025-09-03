#include <iostream>
#include <vector>
#include <string>
#include <unistd.h> // Fork
#include <fcntl.h> // open()
#include <sys/wait.h> // waitpid
using namespace std;

extern pid_t fg_pid;


string strip_quotes(const string &arg) {
    if ((arg[0] == '"' && arg[arg.size() - 1] == '"') ||
        (arg[0] == '\'' && arg[arg.size() - 1] == '\'')) {
        return arg.substr(1, arg.size() - 2);
    }
    return arg;
}

vector<string> tokenize(const string &command) {
    vector<string> tokens;
    string current = "";
    bool inSingle = false, inDouble = false;

    for (size_t i = 0; i < command.size(); i++) {
        char c = command[i];

        // Toggle single-quote mode (only if not inside double quotes)
        if (c == '\'' && !inDouble) {
            inSingle = !inSingle;
            current += c; // keep quotes
            continue;
        }

        // Toggle double-quote mode (only if not inside single quotes)
        if (c == '"' && !inSingle) {
            inDouble = !inDouble;
            current += c; // keep quotes
            continue;
        }

        // Handle spaces (only outside quotes)
        if (!inSingle && !inDouble && c == ' ') {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }

        // Handle redirection operators (only outside quotes)
        if (!inSingle && !inDouble && (c == '<' || c == '>')) {
            if (!current.empty()) {
                current = strip_quotes(current);
                tokens.push_back(current);
                current = "";
            }

            if (c == '>' && i + 1 < command.size() && command[i + 1] == '>') {
                tokens.push_back(">>");
                i++;
            } else {
                string t = "";
                t += c;
                tokens.push_back(t);
            }
            continue;
        }

        // Normal character (or anything inside quotes)
        current += c;
    }

    if (!current.empty()){
        current = strip_quotes(current);
        tokens.push_back(current);
    }

    return tokens;
}


void handle_redirection(const string &command){
    vector<string> tokens = tokenize(command);
    string infile = "", outfile= "";
    bool append = false;
    vector<char*> args;

    // Scan for redirection tokens
    for (int i = 0; i < tokens.size(); i++) {
        if (tokens[i] == "<") {
            if (i + 1 < tokens.size()) infile = strip_quotes(tokens[i + 1]);
            append = false;
            i++;
        }
        else if (tokens[i] == ">") {
            if (i + 1 < tokens.size()) outfile = strip_quotes(tokens[i + 1]);
            append = false;
            i++;
        }
        else if (tokens[i] == ">>") {
            if (i + 1 < tokens.size()) outfile = strip_quotes(tokens[i + 1]);
            append = true;
            i++;
        }
        else {
            args.push_back(const_cast<char*>(tokens[i].c_str()));
        }
    }
    args.push_back(NULL);

    pid_t pid = fork();
    if (pid == 0) {
        // ---- Child ----
        setpgid(0,0);
        if (!infile.empty()) {
            int fd = open(infile.c_str(), O_RDONLY);
            if (fd < 0) {
                perror("Input file error");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (!outfile.empty()) {
            int fd;
            if (append)
                fd = open(outfile.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
            else
                fd = open(outfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);

            if (fd < 0) {
                perror("Output file error");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        if(execvp(args[0], args.data()) < 0){
            perror("execvp");
            exit(1);
        }
    }
    else if (pid > 0) {
        // ---- Parent ----
        fg_pid = pid;
        int status;
        waitpid(pid, &status, WUNTRACED);
        fg_pid = 0;
    }
    else {
        perror("fork");
    }
}