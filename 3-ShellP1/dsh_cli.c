#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dshlib.h"

int main() {
    char cmd_buff[SH_CMD_MAX]; // Buffer for user input
    command_list_t clist;      // Structure to store parsed commands

    while (1) {
        printf("%s", SH_PROMPT); // Display the shell prompt

        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL) { // Read input
            printf("\n"); // Handle EOF (Ctrl+D)
            break;
        }

        cmd_buff[strcspn(cmd_buff, "\n")] = '\0'; // Remove newline

        if (strcmp(cmd_buff, EXIT_CMD) == 0) break; // Handle "exit" command

        int rc = build_cmd_list(cmd_buff, &clist); // Parse command input

        if (rc == OK) {
            printf(CMD_OK_HEADER, clist.num);
            for (int i = 0; i < clist.num; i++) {
                printf("<%d> %s", i + 1, clist.commands[i].exe);
                if (strlen(clist.commands[i].args) > 0) {
                    printf(" [%s]", clist.commands[i].args);
                }
                printf("\n");
            }
        } else if (rc == WARN_NO_CMDS) {
            printf(CMD_WARN_NO_CMD); // Warning for empty input
        } else if (rc == ERR_TOO_MANY_COMMANDS) {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX); // Error for too many pipes
        }
    }

    return 0;
}
