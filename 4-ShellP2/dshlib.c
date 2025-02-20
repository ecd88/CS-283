#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"  // Ensure dshlib.h is included

// Define CMD_ARGV_MAX if not already defined
#ifndef CMD_ARGV_MAX
#define CMD_ARGV_MAX 10  // Set a default value for command arguments
#endif

/*
 * Implements the built-in cd command.
 * - If no arguments are provided, cd does nothing.
 * - If one argument is provided, it attempts to change to the specified directory.
 */
void exec_cd(cmd_buff_t *cmd) {
    if (cmd->argc == 1) {
        return; // No arguments, do nothing
    }
    if (chdir(cmd->argv[1]) != 0) {
        perror("cd failed"); // Print error message if chdir() fails
    }
}

/*
 * Parses user input into cmd_buff_t.
 * - Trims leading and trailing spaces.
 * - Eliminates duplicate spaces (except inside quotes).
 * - Handles quoted strings as single arguments.
 */
void parse_input(char *input, cmd_buff_t *cmd) {
    memset(cmd, 0, sizeof(cmd_buff_t));
    cmd->_cmd_buffer = strdup(input);

    char *ptr = cmd->_cmd_buffer;
    char *token;
    int in_quotes = 0;

    while (*ptr) {
        while (*ptr == ' ') ptr++; // Skip spaces

        if (*ptr == '\0') break;

        if (*ptr == '"') {
            in_quotes = 1;
            ptr++; // Move past the opening quote
            token = ptr;

            while (*ptr && (*ptr != '"' || in_quotes)) {
                if (*ptr == '"') {
                    in_quotes = 0;  // Found closing quote, stop parsing here
                    *ptr = '\0';    // Remove the closing quote
                }
                ptr++;
            }
        } else {
            token = ptr;
            while (*ptr && *ptr != ' ') ptr++;
        }

        if (*ptr) {
            *ptr = '\0'; // Null-terminate token
            ptr++;
        }

        cmd->argv[cmd->argc++] = token;

        if (cmd->argc >= CMD_ARGV_MAX) break;
    }

    cmd->argv[cmd->argc] = NULL;
}

/*
 * Executes external commands using fork() and execvp().
 * - Forks a child process to run the command.
 * - Parent waits for the child to finish.
 * - Handles execution errors.
 */
void exec_external(cmd_buff_t *cmd) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed"); // Forking error
        return;
    }

    if (pid == 0) { // Child process
        execvp(cmd->argv[0], cmd->argv);
        perror("exec failed"); // If execvp() fails, print error
        exit(errno); // Exit with error code
    }

    // Parent process waits for the child
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
    }
}
/*
 * Main loop to process user input and execute commands.
 * - Handles built-in cd command.
 * - Uses exec_external() for other commands.
 * - Returns an integer exit code for main().
 */
int exec_local_cmd_loop() {
    char input[ARG_MAX];
    cmd_buff_t cmd;
    int last_exit_code = 0;

    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(input, ARG_MAX, stdin) == NULL) {
            printf("\n");
            break; // Handle EOF (Ctrl+D)
        }

        input[strcspn(input, "\n")] = '\0'; // Remove newline
        if (strlen(input) == 0) continue; // Ignore empty input

        parse_input(input, &cmd);

        if (cmd.argc == 0) continue; // Ignore empty commands

        if (strcmp(cmd.argv[0], "exit") == 0) break; // Exit shell
        if (strcmp(cmd.argv[0], "cd") == 0) {
            exec_cd(&cmd);
        } else {
            exec_external(&cmd);
        }

        free(cmd._cmd_buffer); // Free allocated memory
    }

    return last_exit_code; // Return exit status for dsh_cli.c
}
