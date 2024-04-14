#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_HISTORY 100
#define MAX_COMMAND_LENGTH 1024

char *history[MAX_HISTORY];
int history_count = 0;

void add_to_history(char *cmd) {
    if (history_count < MAX_HISTORY) {
        history[history_count++] = strdup(cmd);
    } else {
        free(history[0]);
        memmove(history, history + 1, sizeof(char*) * (MAX_HISTORY - 1));
        history[MAX_HISTORY - 1] = strdup(cmd);
    }
}

void print_history() {
    printf("Command History:\n");
    for (int i = 0; i < history_count; ++i) {
        printf("%d: %s\n", i + 1, history[i]);
    }
}

void free_history() {
    for (int i = 0; i < history_count; ++i) {
        free(history[i]);
    }
}

char** parse_command(char* cmd) {
    int space_count = 0;
    for (int i = 0; cmd[i]; i++) {
        if (cmd[i] == ' ') space_count++;
    }

    char **argv = malloc((space_count + 2) * sizeof(char*)); // Plus 2 for the first command and NULL terminator
    int argc = 0;
    char *token = strtok(cmd, " ");
    while (token != NULL) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    argv[argc] = NULL;  // Null-terminate the array
    return argv;
}

int main() {
    char cmd[MAX_COMMAND_LENGTH];
    pid_t pid;
    int status;
    char cwd[1024];

    while (1) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\nsimpleShell> ", cwd);
        } else {
            perror("getcwd() error");
            return 1;
        }
        if (fgets(cmd, sizeof(cmd), stdin) == NULL) {
            break;
        }
        cmd[strcspn(cmd, "\n")] = 0;

        if (strlen(cmd) > 0) {
            add_to_history(cmd);
        }

        if (strcmp(cmd, "exit") == 0) {
            print_history();
            free_history();
            break;
        } else if (strcmp(cmd, "history") == 0) {
            print_history();
            continue;
        }

        char **args = parse_command(cmd);

        if (strcmp(args[0], "cd") == 0) {  // Handle 'cd' command
            if (args[1] == NULL) {
                fprintf(stderr, "cd: expected argument\n");
            } else if (chdir(args[1]) != 0) {
                perror("cd failed");
            }
            free(args);
            continue;
        }

        pid = fork();
        if (pid == 0) {
            execvp(args[0], args);
            fprintf(stderr, "Failed to execute '%s'\n", cmd);
            exit(1);
        } else if (pid > 0) {
            waitpid(pid, &status, 0);
        } else {
            fprintf(stderr, "Failed to fork()\n");
        }
        
        free(args);
    }

    return 0;
}
