#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "dshlib.h"

/**** 
 **** FOR REMOTE SHELL USE YOUR SOLUTION FROM SHELL PART 3 HERE
 **** THE MAIN FUNCTION CALLS THIS ONE AS ITS ENTRY POINT TO
 **** EXECUTE THE SHELL LOCALLY
 ****
 */

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */

void parse_cmd(char *input, command_list_t *clist);
int execute_pipeline(command_list_t *clist);
void exec_command(cmd_buff_t *cmd);

int exec_local_cmd_loop()
{
    char cmd_buff[SH_CMD_MAX];
    command_list_t clist;

    while (1)
    {
    fflush(stdout);
    printf("%s", SH_PROMPT);

        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL)
        {
            if (feof(stdin)) 
            {
                break;
            }
            perror("fgets failed");
            continue;
        }

        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        if (strlen(cmd_buff) == 0)
        {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        if (strcmp(cmd_buff, EXIT_CMD) == 0)
        {
            printf("exiting...\n");
            return OK;
        }

        memset(&clist, 0, sizeof(clist));
        parse_cmd(cmd_buff, &clist);

        if (clist.num == 0)
        {
            printf("Error: No valid commands parsed.\n");
            continue;
        }

        if (strcmp(clist.commands[0].argv[0], "cd") == 0)
        {
            if (clist.commands[0].argc > 1)
            {
                if (chdir(clist.commands[0].argv[1]) != 0)
                {
                    perror("cd failed");
                }
            }
            continue;
        }
        execute_pipeline(&clist);
    }
    return OK;
}

void parse_cmd(char *input, command_list_t *clist)
{
    char *token;
    char *saveptr;
    int cmd_count = 0;

    token = strtok_r(input, PIPE_STRING, &saveptr);
    while (token != NULL)
    {
        if (cmd_count >= CMD_MAX)
        {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            return;
        }

        while (isspace((unsigned char)*token)) token++;  

        clist->commands[cmd_count]._cmd_buffer = strdup(token);
        clist->commands[cmd_count].argc = 0;

        char *arg_token;
        char *arg_saveptr;
        int in_quotes = 0;
        char buffer[CMD_ARGV_MAX * 128] = {0}; 
        int buffer_index = 0;

        arg_token = strtok_r(token, " ", &arg_saveptr);
        while (arg_token != NULL)
        {
            if (arg_token[0] == '"' && arg_token[strlen(arg_token) - 1] == '"')  
            {
                arg_token[strlen(arg_token) - 1] = '\0';  
                clist->commands[cmd_count].argv[clist->commands[cmd_count].argc++] = strdup(arg_token + 1);
            }
            else if (arg_token[0] == '"')  
            {
                in_quotes = 1;
                strcpy(buffer, arg_token + 1);
                buffer_index = strlen(buffer);
            }
            else if (in_quotes)  
            {
                strcat(buffer, " ");
                strcat(buffer, arg_token);
                buffer_index += strlen(arg_token) + 1;

                if (arg_token[strlen(arg_token) - 1] == '"')  
                {
                    buffer[buffer_index - 1] = '\0';  
                    clist->commands[cmd_count].argv[clist->commands[cmd_count].argc++] = strdup(buffer);
                    in_quotes = 0;
                }
            }
            else if (strcmp(arg_token, ">") == 0) 
            {
                arg_token = strtok_r(NULL, " ", &arg_saveptr);
                if (arg_token)
                    clist->commands[cmd_count].outfile = strdup(arg_token);
                clist->commands[cmd_count].append = 0; 
            }
            else if (strcmp(arg_token, ">>") == 0) 
            {
                arg_token = strtok_r(NULL, " ", &arg_saveptr);
                if (arg_token)
                    clist->commands[cmd_count].outfile = strdup(arg_token);
                clist->commands[cmd_count].append = 1; 
            }
            else
            {
                clist->commands[cmd_count].argv[clist->commands[cmd_count].argc++] = strdup(arg_token);
            }

            arg_token = strtok_r(NULL, " ", &arg_saveptr);
        }

        if (in_quotes)
        {
            printf("Error: Unmatched quotes in command.\n");
            return;
        }

        clist->commands[cmd_count].argv[clist->commands[cmd_count].argc] = NULL;
        cmd_count++;
        token = strtok_r(NULL, PIPE_STRING, &saveptr);
    }

    clist->num = cmd_count;
}


int execute_pipeline(command_list_t *clist)
{
    int num_cmds = clist->num;
    int pipes[CMD_MAX - 1][2];
    pid_t pids[CMD_MAX];

    //create pips and fork processes -> redirection
    for (int i = 0; i < num_cmds - 1; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe failed");
            return 1;
        }
    }

    for (int i = 0; i < num_cmds; i++)
    {
        pids[i] = fork();
        if (pids[i] == -1)
        {
            perror("fork failed");
            return 1;
        }

        if (pids[i] == 0) 
        {

            if (clist->commands[i].infile)
            {

                int in_fd = open(clist->commands[i].infile, O_RDONLY);
                if (in_fd < 0)
                {
                    perror("open infile failed");
                    exit(1);
                }
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            if (clist->commands[i].outfile)
            {
                int out_fd = open(clist->commands[i].outfile,
                                  clist->commands[i].append ? (O_WRONLY | O_CREAT | O_APPEND) : (O_WRONLY | O_CREAT | O_TRUNC), 0644);
                if (out_fd < 0)
                {
                    perror("open outfile failed");
                    exit(1);
                }

                if (dup2(out_fd, STDOUT_FILENO) == -1)
                {
                    perror("dup2 failed");
                    exit(1);
                }
                close(out_fd);

        }
            if (i > 0)
            {

                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            if (i < num_cmds - 1)
            {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            for (int j = 0; j < num_cmds - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

        if (clist->commands[i].argv[0] == NULL)
        {
            printf("argv[0] is NULL, command cannot execute!\n");
            exit(1);
        }
          
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp failed");
            exit(errno);
        }
    }

    for (int i = 0; i < num_cmds - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < num_cmds; i++)
    {
        waitpid(pids[i], NULL, 0);
    }

    return 0;
}

