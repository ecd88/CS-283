#ifndef __DSHLIB_H__
#define __DSHLIB_H__

#include <stdio.h>
#include <stdlib.h>

// Define CMD_ARGV_MAX before using it
#ifndef CMD_ARGV_MAX
#define CMD_ARGV_MAX 10  // Set a default value for max command arguments
#endif

// Constants for command structure sizes
#define EXE_MAX 64
#define ARG_MAX 256
#define CMD_MAX 8
// Longest command that can be read from the shell
#define SH_CMD_MAX EXE_MAX + ARG_MAX

typedef struct cmd_buff {
    int argc;
    char *argv[CMD_ARGV_MAX];
    char *_cmd_buffer;
} cmd_buff_t;

typedef struct command {
    char exe[EXE_MAX];
    char args[ARG_MAX];
} command_t;

typedef struct command_list {
    int num;
    command_t commands[CMD_MAX];
} command_list_t;

// Special character #defines
#define SPACE_CHAR ' '
#define PIPE_CHAR '|'
#define PIPE_STRING "|"

#define SH_PROMPT "dsh2> "
#define EXIT_CMD "exit"

// Standard Return Codes
#define OK 0
#define WARN_NO_CMDS -1
#define ERR_TOO_MANY_COMMANDS -2
#define ERR_CMD_OR_ARGS_TOO_BIG -3

// Starter code
#define M_NOT_IMPL "The requested operation is not implemented yet!\n"
#define EXIT_NOT_IMPL 3
#define NOT_IMPLEMENTED_YET 0

// Function prototypes (corrected)
void exec_cd(cmd_buff_t *cmd);         // Built-in `cd` command
void parse_input(char *input, cmd_buff_t *cmd); // Parses input command
void exec_external(cmd_buff_t *cmd);   // Executes external commands
int exec_local_cmd_loop();             // Main command loop

// Output constants
#define CMD_OK_HEADER "PARSED COMMAND LINE - TOTAL COMMANDS %d\n"
#define CMD_WARN_NO_CMD "warning: no commands provided\n"
#define CMD_ERR_PIPE_LIMIT "error: piping limited to %d commands\n"

#endif // __DSHLIB_H__