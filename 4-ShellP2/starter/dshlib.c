#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include "dshlib.h"

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

static int last_rc = 0;

// allocates memory for the command buffer
int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX + 1);
    if(cmd_buff->_cmd_buffer == NULL) {
        return ERR_MEMORY;
    }
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++)
        cmd_buff->argv[i] = NULL;
    return OK;
}

// frees allocated memory in the command buffer
int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if(cmd_buff->_cmd_buffer)
        free(cmd_buff->_cmd_buffer);
    cmd_buff->_cmd_buffer = NULL;
    return OK;
}

// clears fields in cmd_buff for reuse
int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++)
        cmd_buff->argv[i] = NULL;
    // zero out the buffer (optional)
    if(cmd_buff->_cmd_buffer) {
        cmd_buff->_cmd_buffer[0] = '\0';
    }
    return OK;
}

static char *trim_whitespace(char *str) {
    char *end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0)
        return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    *(end+1) = '\0';
    return str;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    // copy cmd_line into the allocated buffer
    strncpy(cmd_buff->_cmd_buffer, cmd_line, SH_CMD_MAX);
    cmd_buff->_cmd_buffer[SH_CMD_MAX] = '\0';
    
    // Trim overall input
    char *line = trim_whitespace(cmd_buff->_cmd_buffer);
    if(strlen(line) == 0)
        return WARN_NO_CMDS;
    
    cmd_buff->argc = 0;
    char *p = line;
    while(*p) {
        // skip spaces
        while(*p && isspace((unsigned char)*p))
            p++;
        if(*p == '\0')
            break;
        // If argument is quoted
        if(*p == '"') {
            p++;  // skip initial quote
            char *start = p;
            // find closing quote
            while(*p && *p != '"')
                p++;
            if(*p == '"') {
                *p = '\0';
                p++; // skip closing quote
            }
            cmd_buff->argv[cmd_buff->argc++] = start;
        } else { // Non-quoted argument
            char *start = p;
            while(*p && !isspace((unsigned char)*p))
                p++;
            if(*p) {
                *p = '\0';
                p++;
            }
            cmd_buff->argv[cmd_buff->argc++] = start;
        }
        if(cmd_buff->argc >= CMD_ARGV_MAX - 1)
            break;
    }
    cmd_buff->argv[cmd_buff->argc] = NULL;  // argv must be null terminated
    return OK;
}

//Match_command: identify if the command is built-in.
Built_In_Cmds match_command(const char *input) {
    if(strcmp(input, EXIT_CMD) == 0)
        return BI_CMD_EXIT;
    else if(strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    else if(strcmp(input, "dragon") == 0)
        return BI_CMD_DRAGON;
    else if(strcmp(input, "rc") == 0)
        return BI_RC;
    return BI_NOT_BI;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    Built_In_Cmds bi = match_command(cmd->argv[0]);
    if(bi == BI_CMD_EXIT) {
        exit(0);
    } else if(bi == BI_CMD_CD) {
        if(cmd->argc < 2) {
            return BI_EXECUTED;
        }
        if(chdir(cmd->argv[1]) != 0) {
            perror("cd");
        } else {
            // Print current working directory after successful cd
            char cwd[1024];
            if(getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);
            }
        }
        return BI_EXECUTED;
    } else if(bi == BI_CMD_DRAGON) {
        extern void print_dragon();
        print_dragon();
        return BI_EXECUTED;
    } else if(bi == BI_RC) {
        // Print last return code from the previous external command.
        printf("%d\n", last_rc);
        return BI_EXECUTED;
    }
    return BI_NOT_BI;
}

int exec_local_cmd_loop() {
    char input_line[SH_CMD_MAX + 1];
    cmd_buff_t cmd;
    int rc = alloc_cmd_buff(&cmd);
    if(rc != OK) {
        fprintf(stderr, "Failed to allocate command buffer\n");
        return ERR_MEMORY;
    }
    while(1) {
        printf("%s", SH_PROMPT);
        if(fgets(input_line, sizeof(input_line), stdin) == NULL) {
            printf("\n");
            break;
        }
        // remove trailing newline and trim overall spaces.
        input_line[strcspn(input_line, "\n")] = '\0';
        char *trimmed = trim_whitespace(input_line);
        if(strlen(trimmed) == 0)
            continue;
        clear_cmd_buff(&cmd);
        rc = build_cmd_buff(trimmed, &cmd);
        if(rc == WARN_NO_CMDS)
            continue;
        if(cmd.argc == 0)
            continue;
        // Check for built-in commands.
        if(match_command(cmd.argv[0]) != BI_NOT_BI) {
            exec_built_in_cmd(&cmd);
            continue;
        }
        // External command, fork & exec
        pid_t pid = fork();
        if(pid < 0) {
            perror("fork");
            continue;
        } else if(pid == 0) { 
            // In child process.
            execvp(cmd.argv[0], cmd.argv);
            // If execvp fails, check errno and print a suitable message.
            if(errno == ENOENT) {
                fprintf(stderr, "Command not found in PATH\n");
            } else if(errno == EACCES) {
                fprintf(stderr, "Permission denied\n");
            } else {
                fprintf(stderr, "execvp error: %s\n", strerror(errno));
            }
            exit(errno);
        } else {
            // Parent process: wait for child and update last_rc with child's exit code.
            int status;
            waitpid(pid, &status, 0);
            if(WIFEXITED(status)) {
                last_rc = WEXITSTATUS(status);
            } else {
                last_rc = -1;
            }
        }
    }
    free_cmd_buff(&cmd);
    return OK;
}