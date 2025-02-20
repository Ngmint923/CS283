#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>  

#include "dshlib.h"

void parse_cmd(char *input, cmd_buff_t *cmd);
void exec_external_cmd(cmd_buff_t *cmd);
void print_dragon(); 

int exec_local_cmd_loop()
{
    char cmd_buff[SH_CMD_MAX];
    cmd_buff_t cmd;
    int rc = OK;

    while (1)
    {
        fflush(stdout);

        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL)
        {
            break;
        }

        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        if (strlen(cmd_buff) == 0)
        {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        if (strcmp(cmd_buff, EXIT_CMD) == 0)
        {
            return OK;
        }

        parse_cmd(cmd_buff, &cmd);
        if (cmd.argc == 0)
            continue;

        if (strcmp(cmd.argv[0], "cd") == 0)
        {
            if (cmd.argc > 1)
            {
                if (chdir(cmd.argv[1]) != 0)
                {
                    perror("cd");
                }
            }
            continue;
        }

        if (strcmp(cmd.argv[0], "dragon") == 0)
        {
            print_dragon();
            continue;
        }

        exec_external_cmd(&cmd);
	printf("%s", SH_PROMPT);
    }



    return OK;
}


void parse_cmd(char *input, cmd_buff_t *cmd)
{
    cmd->argc = 0;
    int in_quotes = 0;
    char buffer[CMD_ARGV_MAX * 128] = {0}; 
    int buffer_index = 0;

    for (int i = 0; input[i] != '\0'; i++)
    {
        if (input[i] == '"')
        {
            in_quotes = !in_quotes;
            continue;
        }

        if (in_quotes || (!isspace(input[i]) && input[i] != '\0'))
        {
            buffer[buffer_index++] = input[i];
        }
        else if (buffer_index > 0)
        {
            buffer[buffer_index] = '\0';
            cmd->argv[cmd->argc++] = strdup(buffer);
            buffer_index = 0;
        }
    }

    if (buffer_index > 0)
    {
        buffer[buffer_index] = '\0';
        cmd->argv[cmd->argc++] = strdup(buffer);
    }

    cmd->argv[cmd->argc] = NULL;
}


void exec_external_cmd(cmd_buff_t *cmd)
{
    pid_t pid = fork();

    if (pid == 0)
    {
        execvp(cmd->argv[0], cmd->argv);
        perror("execvp");
        exit(errno);
    }
    else if (pid > 0) 
    {
        int status;
        waitpid(pid, &status, 0);
        fflush(stdout); 
	printf("%s", SH_PROMPT);
    }
    else
    {
        perror("fork");
    }
}



