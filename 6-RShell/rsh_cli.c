
#include <sys/socket.h>
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
#include <fcntl.h>

#include "dshlib.h"
#include "rshlib.h"

int start_client(char *server_ip, int port){
    int cli_socket;
    struct sockaddr_in server_addr;

    cli_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_socket < 0) {
        perror("socket creation failed");
        return ERR_RDSH_CLIENT;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    if (connect(cli_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        return ERR_RDSH_CLIENT;
    }

    return cli_socket;
}

int exec_remote_cmd_loop(char *address, int port) {
    char *cmd_buff = malloc(RDSH_COMM_BUFF_SZ);
    char *rsp_buff = malloc(RDSH_COMM_BUFF_SZ);
    int cli_socket = start_client(address, port);

    if (!cmd_buff || !rsp_buff || cli_socket < 0) {
        return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_MEMORY);
    }

    while (1) {
        printf("%s", SH_PROMPT);
        fflush(stdout);

        if (fgets(cmd_buff, RDSH_COMM_BUFF_SZ, stdin) == NULL) {
            printf("\n");
            break;
        }

        // Remove trailing newline
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        // Send the command to the server
        if (send(cli_socket, cmd_buff, strlen(cmd_buff) + 1, 0) < 0) {
            perror("send failed");
            break;
        }

        ssize_t recv_size;
        int is_eof = 0;
        while (!is_eof) {
            recv_size = recv(cli_socket, rsp_buff, RDSH_COMM_BUFF_SZ, 0);

            if (recv_size < 0) {
                perror("recv failed");
                break;
            }

            if (recv_size == 0) {
                // Server closed the connection unexpectedly
                printf("\nServer disconnected.\n");
                return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_COMMUNICATION);
            }

            is_eof = (rsp_buff[recv_size - 1] == RDSH_EOF_CHAR);
            if (is_eof) {
                rsp_buff[recv_size - 1] = '\0';  // Replace EOF with null terminator
            }

            printf("%.*s", (int)recv_size, rsp_buff);

            if (is_eof) {
                break;
            }
        }

        // Handle client-side exit
        if (strcmp(cmd_buff, EXIT_CMD) == 0) {
            break;
        }
    }

    return client_cleanup(cli_socket, cmd_buff, rsp_buff, OK);
}

int client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int status) {
    if (cli_socket >= 0) {
        close(cli_socket);
    }
    if (cmd_buff) {
        free(cmd_buff);
    }
    if (rsp_buff) {
        free(rsp_buff);
    }
    return status;
}

