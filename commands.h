#pragma once
#include <string>
using namespace std;

void handle_cd(const string& command);
void handle_pinfo(const string &command);
void handle_search(const string &command);
void handle_redirection(const string &command);
string strip_quotes(const string &command);
void handle_system(const string &command);
void handle_ls(const string &command);
void autocomplete();
void load_history();
void save_history();
void handle_history(const string &arg);
void store_history(const string &arg);
void handle_pipe(const string &command);