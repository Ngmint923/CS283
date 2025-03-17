#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include "dshlib.h"
#include "rshlib.h"

volatile sig_atomic_t server_running = 1;

void handle_sigint(int sig) {
    server_running = 0;
}

int start_server(char *ifaces, int port, int is_threaded){
    int svr_socket;
    int rc;

    signal(SIGINT, handle_sigint);

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0){
        int err_code = svr_socket;
        return err_code;
    }

    rc = process_cli_requests(svr_socket);

    stop_server(svr_socket);

    return rc;
}

int stop_server(int svr_socket){
    return close(svr_socket);
}

int boot_server(char *ifaces, int port){
    int svr_socket;
    int ret;
    struct sockaddr_in addr;

    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket == -1) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }

    int enable = 1;
    setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ifaces);

    ret = bind(svr_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1) {
        perror("bind");
        return ERR_RDSH_COMMUNICATION;
    }

    ret = listen(svr_socket, 20);
    if (ret == -1) {
        perror("listen");
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}

void *thread_client_handler(void *arg) {
    int cli_socket = *(int *)arg;
    free(arg);
    exec_client_requests(cli_socket);
    return NULL;
}

int process_cli_requests(int svr_socket){
    int cli_socket;
    int rc = OK;

    while(server_running){
        cli_socket = accept(svr_socket, NULL, NULL);
        if (cli_socket < 0) {
            perror("accept");
            return ERR_RDSH_COMMUNICATION;
        }

        pthread_t tid;
        int *pclient = malloc(sizeof(int));
        *pclient = cli_socket;
        pthread_create(&tid, NULL, thread_client_handler, pclient);
        pthread_detach(tid);
    }

    return rc;
}

iint exec_client_requests(int cli_socket) {
    char *io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (!io_buff) return ERR_RDSH_SERVER;

    while (1) {
        int io_size = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ, 0);
        if (io_size <= 0) {
            break;  // Client disconnected or error occurred.
        }

        io_buff[io_size] = '\0';

        if (strcmp(io_buff, "stop-server") == 0) {
            send_message_eof(cli_socket);  
            close(cli_socket);             
            free(io_buff);
            return OK_EXIT;
        } 
        else if (strcmp(io_buff, "exit") == 0) {
            send_message_eof(cli_socket);  
            close(cli_socket);             
            free(io_buff);
            return OK;
        } 
        else {
            command_list_t cmd_list;
            build_cmd_list(io_buff, &cmd_list);    
            rsh_execute_pipeline(cli_socket, &cmd_list);
            send_message_eof(cli_socket);          
        }
    }

    free(io_buff);
    return OK;
}



int send_message_string(int cli_socket, char *buff){
    int send_len = strlen(buff) + 1;
    int sent_len = send(cli_socket, buff, send_len, 0);
    if (sent_len != send_len) {
        return ERR_RDSH_COMMUNICATION;
    }
    return send_message_eof(cli_socket);
}

int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int pipes[clist->num - 1][2];
    pid_t pids[clist->num];
    int pids_st[clist->num];
    int exit_code;

    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            if (i == 0) dup2(cli_sock, STDIN_FILENO);
            if (i == clist->num - 1) {
                dup2(cli_sock, STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
            }
            if (i > 0) dup2(pipes[i - 1][0], STDIN_FILENO);
            if (i < clist->num - 1) dup2(pipes[i][1], STDOUT_FILENO);

            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], &pids_st[i], 0);
    }

    exit_code = WEXITSTATUS(pids_st[clist->num - 1]);
    return exit_code;
}



Built_In_Cmds rsh_match_command(const char *input) {
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

Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd) {
    Built_In_Cmds ctype = rsh_match_command(cmd->argv[0]);

    switch (ctype) {
    case BI_CMD_EXIT:
        return BI_CMD_EXIT;
    case BI_CMD_STOP_SVR:
        return BI_CMD_STOP_SVR;
    case BI_CMD_CD:
        chdir(cmd->argv[1]);
        return BI_EXECUTED;
    default:
        return BI_NOT_BI;
    }
}

