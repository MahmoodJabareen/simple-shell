# Simple Shell

This project implements a simple shell in C, designed to provide basic command execution capabilities along with process management and command history features.

## Overview

The simple shell allows users to execute commands, manage processes, and utilize input/output redirection. It supports internal commands such as `cd`, `history`, and process management commands like `suspend`, `blast`, and `alarm`. The shell maintains a history of executed commands, allowing users to re-execute previous commands easily.

## Features

- Execute external commands using `execvp`.
- Manage a list of processes, including their statuses (running, suspended, terminated).
- Input/output redirection for commands.
- Command history management, allowing users to view and re-execute previous commands.
- Internal commands support, including:
  - `cd`: Change the current directory.
  - `history`: Display the command history.
  - `suspend`: Suspend a process.
  - `blast`: Terminate a process.
  - `alarm`: Resume a suspended process.

## Compilation

To compile the shell, navigate to the `simple-shell/shell` directory and run the following command:

```bash
gcc -o myshell myshell.c -Wall
```

## Running the Shell

After compiling, you can run the shell by executing:

```bash
./myshell
```

You will be presented with a prompt where you can enter commands.

## Usage

- Type your command and press Enter to execute it.
- Use `cd <directory>` to change directories.
- Use `history` to view the command history.
- Use `!<number>` to execute a command from history.
- Use `!!` to execute the last command again.
- Use `procs` to view the list of currently running processes.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.