#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> // O_RDONLY, O_WRONLY, O_CREAT, O_TRUNC
#include "LineParser.h"
#include <signal.h>

void execute(cmdLine *pCmdLine, int debugMode) {
    pid_t pid = fork();
    if (pid == -1) {//error
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {

        if (pCmdLine->inputRedirect) {
            close(STDIN_FILENO);
            if (open(pCmdLine->inputRedirect, O_RDONLY) != STDIN_FILENO) {
                perror("open inputRedirect");
                _exit(EXIT_FAILURE);
            }
        }

        if (pCmdLine->outputRedirect) {
            close(STDOUT_FILENO);
            if (open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0644) != STDOUT_FILENO) {
                perror("open outputRedirect");
                _exit(EXIT_FAILURE);
            }
        }
        if (debugMode) {
            fprintf(stderr, "PID: %d\n", getpid());
            fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
        }
        execvp(pCmdLine->arguments[0], pCmdLine->arguments);
        perror("execvp");
        _exit(EXIT_FAILURE); 
    } else {
        int status;
        if (pCmdLine->blocking) {
            waitpid(pid, &status, 0);
        }
    }
}


void handle_internal_commands(cmdLine *pCmdLine) {
    if (strcmp(pCmdLine->arguments[0], "cd") == 0) {
        if (pCmdLine->argCount < 2) {
            fprintf(stderr, "cd: missing argument\n");
        } else {
            if (chdir(pCmdLine->arguments[1]) == -1) {
                perror("chdir");
            }
        }
    } else if (strcmp(pCmdLine->arguments[0], "alarm") == 0) {
        if (pCmdLine->argCount < 2) {
            fprintf(stderr, "alarm: missing process id\n");
        } else {
            pid_t pid = atoi(pCmdLine->arguments[1]);
            if (kill(pid, SIGCONT) == -1) {
                perror("kill SIGCONT");
            } else {
                printf("Sent SIGCONT to process %d\n", pid);
            }
        }
    } else if (strcmp(pCmdLine->arguments[0], "blast") == 0) {
        if (pCmdLine->argCount < 2) {
            fprintf(stderr, "blast: missing process id\n");
        } else {
            pid_t pid = atoi(pCmdLine->arguments[1]);
            if (kill(pid, SIGKILL) == -1) {
                perror("kill SIGKILL");
            } else {
                printf("Sent SIGKILL to process %d\n", pid);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    char cwd[PATH_MAX];
    char input[2048];
    cmdLine *parsedLine;
    int debugMode = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            debugMode = 1;
        }
    }

    while (1) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s> ", cwd);
        } else {
            perror("getcwd");
            exit(EXIT_FAILURE);
        }

        if (fgets(input, sizeof(input), stdin) == NULL) {
            perror("fgets");
            continue;
        }

        input[strcspn(input, "\n")] = '\0';

        parsedLine = parseCmdLines(input);
        if (parsedLine == NULL) {
            continue;
        }

        if (strcmp(parsedLine->arguments[0], "quit") == 0) {
            freeCmdLines(parsedLine);
            break;
        }

        if (strcmp(parsedLine->arguments[0], "cd") == 0 ||
            strcmp(parsedLine->arguments[0], "alarm") == 0 ||
            strcmp(parsedLine->arguments[0], "blast") == 0) {
            handle_internal_commands(parsedLine);
            freeCmdLines(parsedLine);
            continue;
        }

        execute(parsedLine, debugMode);

        freeCmdLines(parsedLine);
    }
    return 0;
}
