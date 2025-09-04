# Custom POSIX Shell (Assignment 2 – Advanced Operating Systems)

## Overview
This project is a **custom shell implementation in C++** that mimics the behavior of standard shells like `bash` or `zsh`. It supports built-in commands (`cd`, `pwd`, `ls`, etc.), execution of system commands, background/foreground jobs, input/output redirection, pipelines, command history, autocomplete, and signal handling.

The shell was implemented as part of **Assignment 2 for the Advanced Operating Systems (Monsoon 2025)** course.

---

## Features Implemented
1. **Display Prompt**
   - Format: `<username>@<system_name>:<current_directory>$`
   - Supports `~` for home directory.

2. **Built-in Commands**
   - `cd` – supports `.`, `..`, `-`, `~` and error handling for multiple arguments.
   - `pwd` – prints the current working directory.
   - `echo` – handles quotes and multiple spaces/tabs.
   - `ls` – supports `-a`, `-l`, multiple flags, and multiple directory arguments.
   - `pinfo` – displays process info for given PID or current shell.
   - `search` – recursively searches for a file/directory from the current path.
   - `history` – maintains last 20 commands across sessions, supports `history <n>`.

3. **System Commands**
   - Executes external programs via `execvp`.
   - Supports **foreground** and **background** (`&`) execution.

4. **Redirection**
   - Handles `<`, `>`, `>>` operators for input/output redirection.

5. **Pipelines**
   - Handles `|` operator across multiple commands.
   - Works with redirection inside pipelines.

6. **Signals**
   - `CTRL+C` – interrupts foreground job.
   - `CTRL+Z` – pushes foreground job to background (stopped state).
   - `CTRL+D` – logs out of the shell gracefully.

7. **History Navigation**
   - `↑` and `↓` arrows navigate through history commands (modifiable before execution).

8. **Autocomplete**
   - Pressing `TAB` autocompletes commands or files in the current directory.

---

## File Structure
```
2025201008_Assignment2/
│── README.md              # Documentation (this file)
│── makefile               # Build instructions
│── main.cpp               # Entry point, shell loop, signal handling
│── cd.cpp                 # Implementation of 'cd'
│── ls.cpp                 # Implementation of 'ls'
│── search.cpp             # Recursive file search
│── history.cpp            # Command history management
│── autocomplete.cpp       # Tab completion logic
│── pinfo.cpp              # Process info implementation
│── redirection.cpp        # Handles input/output redirection
│── pipe.cpp               # Handles command pipelines
│── back-foreground.cpp    # Background/foreground process management
│── commands.h             # Header for function declarations
```

---

## Compilation & Execution
1. Run `make` to compile all files:
   ```bash
   make
   ```

2. Start the shell:
   ```bash
   ./myshell
   ```

3. To clean build files:
   ```bash
   make clean
   ```

---

## Example Usage
```bash
<Name@UBUNTU:~> ls -l -a
<Name@UBUNTU:~> cd test
<Name@UBUNTU:~/test> echo "hello world" > file.txt
<Name@UBUNTU:~/test> cat < file.txt | wc -l
<Name@UBUNTU:~/test> gedit &
<Name@UBUNTU:~/test> pinfo
<Name@UBUNTU:~/test> history 5
<Name@UBUNTU:~/test> exit
```
