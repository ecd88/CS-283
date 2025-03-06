#ifndef __DSHLIB_H__
#define __DSHLIB_H__

#include <stdio.h>
#include <stdlib.h>

// Define CMD_ARGV_MAX before using it
#ifndef CMD_ARGV_MAX
#define CMD_ARGV_MAX 10  // Max command arguments
#endif

// Constants for command structure sizes
#define EXE_MAX 64
#define ARG_MAX 256
#define CMD_MAX 8  // Max number of piped commands
#define SH_CMD_MAX EXE_MAX + ARG_MAX

typedef struct cmd_buff {
    int argc;
    char *argv[CMD_ARGV_MAX];
    char *_cmd_buffer;
} cmd_buff_t;

// Special character defines
#define PIPE_CHAR '|'
#define PIPE_STRING "|"

#define SH_PROMPT "dsh3> "
#define EXIT_CMD "exit"

// Function prototypes
void exec_cd(cmd_buff_t *cmd);
void parse_input(char *input, cmd_buff_t cmds[], int *cmd_count);
void exec_external(cmd_buff_t *cmd);
void execute_piped_commands(cmd_buff_t cmds[], int cmd_count);
int exec_local_cmd_loop();

#endif // __DSHLIB_H__

