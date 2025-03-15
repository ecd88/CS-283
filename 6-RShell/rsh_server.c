
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

#include "dshlib.h"
#include "rshlib.h"


#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "dshlib.h"
#include "rshlib.h"

#define MAX_COMMANDS 10
#define MAX_ARGS 10

int free_cmd_list(command_list_t *clist) {
    if (!clist) return ERR_MEMORY;

    for (int i = 0; i < clist->num; i++) {
        for (int j = 0; j < clist->commands[i].argc; j++) {
            free(clist->commands[i].argv[j]);
        }
    }

    clist->num = 0;
    return OK;
}


int start_server(char *ifaces, int port, int is_threaded){
    int svr_socket;
    int rc;

    //
    //TODO:  If you are implementing the extra credit, please add logic
    //       to keep track of is_threaded to handle this feature
    //

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0){
        int err_code = svr_socket;  //server socket will carry error code
        return err_code;
    }

    rc = process_cli_requests(svr_socket);

    stop_server(svr_socket);


    return rc;
}


int stop_server(int svr_socket){
    return close(svr_socket);
}


int boot_server(char *ifaces, int port) {
    int svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket < 0) {
        perror("socket creation failed");
        return ERR_RDSH_COMMUNICATION;
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
    };
    inet_pton(AF_INET, ifaces, &server_addr.sin_addr);

    int opt = 1;
    setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(svr_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        return ERR_RDSH_COMMUNICATION;
    }

    if (listen(svr_socket, 10) < 0) {
        perror("listen failed");
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}

int process_cli_requests(int svr_socket) {
    while (1) {
        int cli_socket = accept(svr_socket, NULL, NULL);
        if (cli_socket < 0) {
            perror("accept failed");
            return ERR_RDSH_COMMUNICATION;
        }
        int rc = exec_client_requests(cli_socket);
        close(cli_socket);
        if (rc == OK_EXIT) break;
    }
    stop_server(svr_socket);
    return OK;
}

int build_cmd_list(char *input, command_list_t *clist) {
    char *command_str;
    char *arg;
    int command_count = 0;

    // Split the input by '|'
    command_str = strtok(input, "|");
    while (command_str != NULL && command_count < MAX_COMMANDS) {
        int arg_count = 0;

        // Trim leading and trailing spaces
        while (*command_str == ' ') command_str++;
        char *end = command_str + strlen(command_str) - 1;
        while (end > command_str && *end == ' ') *end-- = '\0';

        // Tokenize the command arguments
        arg = strtok(command_str, " ");
        while (arg != NULL && arg_count < MAX_ARGS) {
            clist->commands[command_count].argv[arg_count++] = strdup(arg); // strdup to allocate dynamically
            arg = strtok(NULL, " ");
        }
        clist->commands[command_count].argv[arg_count] = NULL;
        clist->commands[command_count].argc = arg_count;
        command_count++;

        // Next command
        command_str = strtok(NULL, "|");
    }

    clist->num = command_count;
    return OK;
}




int exec_client_requests(int cli_socket) {
    char *io_buff = malloc(RDSH_COMM_BUFF_SZ);
    command_list_t clist;
    int rc = OK;

    if (!io_buff) return ERR_MEMORY;

    while (1) {
        memset(io_buff, 0, RDSH_COMM_BUFF_SZ);

        ssize_t recv_size = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ, 0);
        
        // Only break if the client has closed the connection
        if (recv_size == 0) {
            printf("Client has disconnected.\n");
            rc = OK;
            break;
        } else if (recv_size < 0) {
            perror("recv failed on server");
            rc = ERR_RDSH_COMMUNICATION;
            break;
        }

        memset(&clist, 0, sizeof(command_list_t));

        if (strcmp(io_buff, "exit") == 0) {
            rc = OK;
            break;
        } else if (strcmp(io_buff, "stop-server") == 0) {
            rc = OK_EXIT;
            break;
        } else {
            command_list_t cmd_list;
            build_cmd_list(io_buff, &cmd_list);
            rsh_execute_pipeline(cli_socket, &cmd_list);
            send_message_eof(cli_socket);
        }
    }

    free(io_buff);
    return rc;
}



int send_message_eof(int cli_socket){
    int send_len = 1;
    int sent_len = send(cli_socket, &RDSH_EOF_CHAR, send_len, 0);
    
    if (sent_len != send_len) {
        perror("send_message_eof failed");
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}

int send_message_string(int cli_socket, char *buff){
    ssize_t sent_len = send(cli_socket, buff, strlen(buff), 0);
    if (sent_len < 0) {
        perror("send failed");
        return ERR_RDSH_COMMUNICATION;
    }
    return send_message_eof(cli_socket);
}

int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int pipes[clist->num - 1][2];
    pid_t pids[clist->num];
    int pids_st[clist->num];
    int exit_code;

    // Create necessary pipes
    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe creation failed");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {  // Child process
            if (i == 0) dup2(cli_sock, STDIN_FILENO);
            if (i == clist->num - 1) {
                dup2(cli_sock, STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
            }
            // Redirect stdin for the first or intermediate processes
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            // Redirect stdout for intermediate or last process
            if (i < clist->num - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            } else {
                // Last process output goes to the client socket
                dup2(cli_sock, STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
            }

            // Close all pipes in the child process
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Execute the command
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
    }

    // Parent process: Close all pipe ends
    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all child processes
    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], &pids_st[i], 0);
    }

    // Capture the exit code of the last process
    exit_code = WEXITSTATUS(pids_st[clist->num - 1]);

    return exit_code;
}



Built_In_Cmds rsh_match_command(const char *input)
{
    if (strcmp(input, "exit") == 0)
        return BI_CMD_EXIT;
    if (strcmp(input, "dragon") == 0)
        return BI_CMD_DRAGON;
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    if (strcmp(input, "stop-server") == 0)
        return BI_CMD_STOP_SVR;
    if (strcmp(input, "rc") == 0)
        return BI_CMD_RC;
    return BI_NOT_BI;
}


Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd)
{
    Built_In_Cmds ctype = BI_NOT_BI;
    ctype = rsh_match_command(cmd->argv[0]);

    switch (ctype)
    {
    // case BI_CMD_DRAGON:
    //     print_dragon();
    //     return BI_EXECUTED;
    case BI_CMD_EXIT:
        return BI_CMD_EXIT;
    case BI_CMD_STOP_SVR:
        return BI_CMD_STOP_SVR;
    case BI_CMD_RC:
        return BI_CMD_RC;
    case BI_CMD_CD:
        chdir(cmd->argv[1]);
        return BI_EXECUTED;
    default:
        return BI_NOT_BI;
    }
}
