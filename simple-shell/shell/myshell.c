#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <limits.h>
#include "LineParser.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <linux/limits.h>
#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0
#define HISTLEN 20
#define MAX_BUF 200

typedef struct process
{
    cmdLine *cmd;
    pid_t pid;
    int status;
    struct process *next;
} process;

process *process_list_singelton = NULL;
char history[HISTLEN][MAX_BUF];
int newest = 0;
int oldest = 0;
int history_count = 0;

void addProcess(process **process_list, cmdLine *cmd, pid_t pid);
void printProcessList(process **process_list);
void deleteTerminatedProcess(process **process_list, process *proc);
void freeProcessList(process *process_list);
void updateProcessList(process **process_list);
void updateProcessStatus(process *process_list, int pid, int status);
void addHistory(const char *cmd);
void printHistory();
void executeHistoryCommand(char history[HISTLEN][MAX_BUF], int index);
void handle_internal_commands(cmdLine *pCmdLine);
void execute(cmdLine *pCmdLine, int debugMode);
void executePipe(cmdLine *commandLine);

void addProcess(process **process_list, cmdLine *cmd, pid_t pid)
{
    process *new_process = (process *)malloc(sizeof(process));
    new_process->cmd = cmd;
    new_process->pid = pid;
    new_process->status = RUNNING;
    new_process->next = *process_list;
    *process_list = new_process;
}

void printProcessList(process **process_list)
{
    updateProcessList(process_list);
    printf("PID\t\tCommand\t\tSTATUS\n");
    process *current = *process_list;
    process *prev = NULL;
    while (current != NULL)
    {
        printf("%d\t\t%s\t\t%s\n", current->pid, current->cmd->arguments[0],
               (current->status == RUNNING ? "Running" : (current->status == SUSPENDED ? "Suspended" : "Terminated")));
        if (current->status == TERMINATED)
        {
            if (prev == NULL)
            {
                *process_list = current->next;
            }
            else
            {
                prev->next = current->next;
            }
            process *temp = current;
            current = current->next;
            freeCmdLines(temp->cmd);
            free(temp);
        }
        else
        {
            prev = current;
            current = current->next;
        }
    }
}

void deleteTerminatedProcess(process **process_list, process *proc)
{
    if (*process_list == NULL || proc == NULL)
        return;
    if (*process_list == proc)
    {
        *process_list = proc->next;
        freeCmdLines(proc->cmd);
        free(proc);
        return;
    }
    process *prev = *process_list;
    while (prev->next != NULL && prev->next != proc)
    {
        prev = prev->next;
    }
    if (prev->next == NULL)
        return;
    prev->next = proc->next;
    freeCmdLines(proc->cmd);
    free(proc);
}

void freeProcessList(process *process_list)
{
    while (process_list != NULL)
    {
        process *temp = process_list;
        process_list = process_list->next;
        freeCmdLines(temp->cmd);
        free(temp);
    }
}

void updateProcessList(process **process_list)
{
    process *current = *process_list;
    while (current != NULL)
    {
        int status;
        pid_t result = waitpid(current->pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
        if (result == 0)
        {
        }
        else if (result == -1)
        {
            perror("waitpid");
        }
        else
        {
            if (WIFEXITED(status) || WIFSIGNALED(status))
            {
                current->status = TERMINATED;
            }
            else if (WIFSTOPPED(status))
            {
                current->status = SUSPENDED;
            }
            else if (WIFCONTINUED(status))
            {
                current->status = RUNNING;
            }
        }
        current = current->next;
    }
}

void updateProcessStatus(process *process_list, int pid, int status)
{
    if (process_list == NULL)
        return;
    process *cur = process_list;
    while (cur != NULL)
    {
        if (cur->pid == pid)
        {
            cur->status = status;
            return;
        }
        cur = cur->next;
    }
}

bool debug = false;

void execute(cmdLine *pCmdLine, int debugMode)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        if (pCmdLine->inputRedirect)
        {
            close(STDIN_FILENO);
            if (open(pCmdLine->inputRedirect, O_RDONLY) != STDIN_FILENO)
            {
                perror("open inputRedirect");
                _exit(EXIT_FAILURE);
            }
        }
        if (pCmdLine->outputRedirect)
        {
            close(STDOUT_FILENO);
            if (open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0644) != STDOUT_FILENO)
            {
                perror("open outputRedirect");
                _exit(EXIT_FAILURE);
            }
        }
        if (debugMode)
        {
            fprintf(stderr, "PID: %d\n", getpid());
            fprintf(stderr, "Executing command : %s\n", pCmdLine->arguments[0]);
        }
        execvp(pCmdLine->arguments[0], pCmdLine->arguments);
        perror("execvp");
        _exit(EXIT_FAILURE);
    }
    addProcess(&process_list_singelton, pCmdLine, pid);
    int status;
    if (pCmdLine->blocking)
    {
        waitpid(pid, &status, 0);
    }
}

void executePipe(cmdLine *commandLine)
{
    int mypipe[2];
    pid_t child1;
    pid_t child2;
    if (pipe(mypipe) == -1)
        perror("ERROR CREATING A PIPE ");
    child1 = fork();
    if (child1 == -1)
    {
        perror("ERROR FORKING");
        _exit(EXIT_FAILURE);
    }
    if (child1 == 0)
    {
        close(STDOUT_FILENO);
        if (dup(mypipe[1]) == -1)
        {
            perror("ERROR DUPLICATING ");
            _exit(EXIT_FAILURE);
        }
        if (close(mypipe[1]) == -1)
        {
            perror("ERROR CLOSING FILE DESCRIPTOR");
            _exit(EXIT_FAILURE);
        }
        if (execvp(commandLine->arguments[0], commandLine->arguments) == -1)
        {
            perror("ERROR EXECUTING");
            _exit(EXIT_FAILURE);
        }
    }
    else
    {
        addProcess(&process_list_singelton, commandLine, child1);
        if (close(mypipe[1]) == -1)
        {
            perror("ERROR CLOSING WRITE END OF PIPE");
            _exit(EXIT_FAILURE);
        }
        child2 = fork();
        if (child2 == -1)
        {
            perror("ERROR FORKING CHILD2");
            _exit(EXIT_FAILURE);
        }
        if (child2 == 0)
        {
            close(STDIN_FILENO);
            if (dup(mypipe[0]) == -1)
            {
                perror("ERROR DUPLICATING ");
                _exit(EXIT_FAILURE);
            }
            if (close(mypipe[0]) == -1)
            {
                perror("ERROR CLOSING FILE DESCRIPTOR");
                _exit(EXIT_FAILURE);
            }
            if (execvp(commandLine->next->arguments[0], commandLine->next->arguments) == -1)
            {
                perror("ERROR EXECUTING");
                _exit(EXIT_FAILURE);
            }
        }
        else
        {
            addProcess(&process_list_singelton, commandLine->next, child2);
            if (close(mypipe[0]) == -1)
            {
                perror("ERROR CLOSING FILE DESCRIPTOR");
                _exit(EXIT_FAILURE);
            }
            waitpid(child1, NULL, 0);
            waitpid(child2, NULL, 0);
        }
    }
}

void addHistory(const char *cmd)
{
    strncpy(history[newest], cmd, MAX_BUF);
    history[newest][MAX_BUF - 1] = '\0';
    newest = (newest + 1) % HISTLEN;
    if (history_count < HISTLEN)
    {
        history_count++;
    }
    else
    {
        oldest = (oldest + 1) % HISTLEN;
    }
    printf("Added to history : %s\n", cmd);
}

void printHistory()
{
    printf("History:\n");
    int i = oldest;
    int count = 0;
    while (count < history_count)
    {
        printf("%d: %s\n", count + 1, history[i]);
        i = (i + 1) % HISTLEN;
        count++;
    }
}

void executeHistoryCommand(char history[HISTLEN][MAX_BUF], int index)
{
    if (index < 1 || index > history_count)
    {
        fprintf(stderr, "Error: Invalid history index.\n");
        return;
    }
    int commandIndex = (oldest + index - 1) % HISTLEN;
    char commandLine[MAX_BUF];
    strncpy(commandLine, history[commandIndex], MAX_BUF - 1);
    commandLine[MAX_BUF - 1] = '\0';
    printf("Executing command: %s\n", commandLine);
    cmdLine *parsedLine = parseCmdLines(commandLine);
    if (!parsedLine)
    {
        fprintf(stderr, "Error: Failed to parse command.\n");
        return;
    }
    if (strcmp(parsedLine->arguments[0], "cd") == 0 ||
        strcmp(parsedLine->arguments[0], "alarm") == 0 ||
        strcmp(parsedLine->arguments[0], "blast") == 0 ||
        strcmp(parsedLine->arguments[0], "suspend") == 0 ||
        strcmp(parsedLine->arguments[0], "sleep") == 0 ||
        strcmp(parsedLine->arguments[0], "procs") == 0 ||
        strcmp(parsedLine->arguments[0], "history") == 0 ||
        strcmp(parsedLine->arguments[0], "!!") == 0 ||
        parsedLine->arguments[0][0] == '!')
    {
        handle_internal_commands(parsedLine);
        freeCmdLines(parsedLine);
        return;
    }
    else
    {
        execute(parsedLine, 1);
    }
    if (parsedLine->next != NULL)
    {
        executePipe(parsedLine);
    }
    freeCmdLines(parsedLine);
}

void handle_internal_commands(cmdLine *pCmdLine)
{
    if (strcmp(pCmdLine->arguments[0], "history") == 0)
    {
        printHistory();
    }
    else if (strcmp(pCmdLine->arguments[0], "!!") == 0)
    {
        if (history_count > 0)
        {
            executeHistoryCommand(history, history_count - 1);
        }
    }
    else if (pCmdLine->arguments[0][0] == '!')
    {
        int index = atoi(pCmdLine->arguments[0] + 1);
        executeHistoryCommand(history, index);
    }
    else if (strcmp(pCmdLine->arguments[0], "cd") == 0)
    {
        if (pCmdLine->argCount < 2)
        {
            fprintf(stderr, "cd: missing argument\n");
        }
        else
        {
            if (chdir(pCmdLine->arguments[1]) == -1)
            {
                perror("chdir");
            }
        }
    }
    else if (strcmp(pCmdLine->arguments[0], "procs") == 0)
    {
        printProcessList(&process_list_singelton);
    }
    else if (strcmp(pCmdLine->arguments[0], "sleep") == 0 && pCmdLine->argCount > 1)
    {
        pid_t pid = atoi(pCmdLine->arguments[1]);
        if (kill(pid, SIGTSTP) == 0)
        {
            updateProcessStatus(process_list_singelton, pid, SUSPENDED);
        }
        else
        {
            perror("kill");
        }
    }
    else if (strcmp(pCmdLine->arguments[0], "blast") == 0 && pCmdLine->argCount > 1)
    {
        pid_t pid = atoi(pCmdLine->arguments[1]);
        if (kill(pid, SIGINT) == 0)
        {
            updateProcessStatus(process_list_singelton, pid, TERMINATED);
        }
        else
        {
            perror("kill");
        }
    }
    else if (strcmp(pCmdLine->arguments[0], "alarm") == 0 && pCmdLine->argCount > 1)
    {
        pid_t pid = atoi(pCmdLine->arguments[1]);
        if (kill(pid, SIGCONT) == 0)
        {
            updateProcessStatus(process_list_singelton, pid, RUNNING);
        }
        else
        {
            perror("kill");
        }
    }
    else if (strcmp(pCmdLine->arguments[0], "wake") == 0)
    {
        if (pCmdLine->argCount < 2)
        {
            fprintf(stderr, "wake: missing process id\n");
        }
        else
        {
            pid_t pid = atoi(pCmdLine->arguments[1]);
            if (kill(pid, SIGCONT) == -1)
            {
                perror("kill SIGCONT");
            }
            else
            {
                printf("Sent SIGCONT to process %d\n", pid);
                updateProcessStatus(process_list_singelton, pid, RUNNING);
            }
        }
    }
    else if (strcmp(pCmdLine->arguments[0], "suspend") == 0)
    {
        if (pCmdLine->argCount < 2)
        {
            fprintf(stderr, "suspend: missing process id\n");
        }
        else
        {
            pid_t pid = atoi(pCmdLine->arguments[1]);
            if (kill(pid, SIGTSTP) == -1)
            {
                perror("kill SIGTSTP");
            }
            else
            {
                printf("Sent SIGTSTP to process %d\n", pid);
                updateProcessStatus(process_list_singelton, pid, SUSPENDED);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    char cwd[PATH_MAX];
    char input[2048];
    cmdLine *parsedLine;
    int debugMode = 0;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-d") == 0)
        {
            debugMode = 1;
        }
    }
    while (1)
    {
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s> ", cwd);
        }
        else
        {
            perror("getcwd");
            exit(EXIT_FAILURE);
        }

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            perror("fgets");
            continue;
        }

        input[strcspn(input, "\n")] = '\0';
        parsedLine = parseCmdLines(input);
        if (parsedLine == NULL)
        {
            continue;
        }

        addHistory(input);

        if (strcmp(parsedLine->arguments[0], "quit") == 0)
        {
            freeCmdLines(parsedLine);
            break;
        }

        if (strcmp(parsedLine->arguments[0], "cd") == 0 ||
            strcmp(parsedLine->arguments[0], "alarm") == 0 ||
            strcmp(parsedLine->arguments[0], "blast") == 0 ||
            strcmp(parsedLine->arguments[0], "suspend") == 0 ||
            strcmp(parsedLine->arguments[0], "sleep") == 0 ||
            strcmp(parsedLine->arguments[0], "procs") == 0 ||
            strcmp(parsedLine->arguments[0], "history") == 0 ||
            strcmp(parsedLine->arguments[0], "!!") == 0 ||
            parsedLine->arguments[0][0] == '!')
        {
            handle_internal_commands(parsedLine);
            freeCmdLines(parsedLine);
            continue;
        }
        execute(parsedLine, debugMode);

        if (parsedLine->next != NULL)
        {
            executePipe(parsedLine);
        }
    }
    freeProcessList(process_list_singelton);
    return 0;
}