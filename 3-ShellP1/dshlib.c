#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "dshlib.h"

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    if (!cmd_line || !clist) return ERR_CMD_OR_ARGS_TOO_BIG;

    memset(clist, 0, sizeof(command_list_t)); // Initialize structure

    char *saveptr_cmd;
    char *cmd_token = strtok_r(cmd_line, PIPE_STRING, &saveptr_cmd); // Split commands by '|'

    while (cmd_token != NULL && clist->num < CMD_MAX) {
        command_t *cmd = &clist->commands[clist->num];

        while (*cmd_token == SPACE_CHAR) cmd_token++; // Trim leading spaces

        char *saveptr_arg;
        char *arg_token = strtok_r(cmd_token, " ", &saveptr_arg); // Extract command name
        if (!arg_token) break;

        strncpy(cmd->exe, arg_token, EXE_MAX - 1);
        cmd->exe[EXE_MAX - 1] = '\0';

        cmd->args[0] = '\0';
        while ((arg_token = strtok_r(NULL, " ", &saveptr_arg)) != NULL) { // Parse arguments
            if (strlen(cmd->args) + strlen(arg_token) + 2 < ARG_MAX) {
                if (strlen(cmd->args) > 0) strncat(cmd->args, " ", ARG_MAX - strlen(cmd->args) - 1);
                strncat(cmd->args, arg_token, ARG_MAX - strlen(cmd->args) - 1);
            } else {
                return ERR_CMD_OR_ARGS_TOO_BIG; // Argument too long
            }
        }

        clist->num++;
        cmd_token = strtok_r(NULL, PIPE_STRING, &saveptr_cmd); // Get next command
    }

    if (clist->num == 0) return WARN_NO_CMDS; // No valid commands found
    if (cmd_token != NULL) return ERR_TOO_MANY_COMMANDS; // Exceeded command limit

    return OK;
}
