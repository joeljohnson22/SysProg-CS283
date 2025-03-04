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
    
    // Free redirection paths
    if(cmd_buff->input_file)
        free(cmd_buff->input_file);
    if(cmd_buff->output_file)
        free(cmd_buff->output_file);
        
    cmd_buff->input_file = NULL;
    cmd_buff->output_file = NULL;
    
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
    
    // Clear redirection fields
    cmd_buff->input_file = NULL;
    cmd_buff->output_file = NULL;
    cmd_buff->append_mode = 0;
    
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
    
    // Initialize redirection fields
    cmd_buff->input_file = NULL;
    cmd_buff->output_file = NULL;
    cmd_buff->append_mode = 0;
    
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
        
        // Check for redirection operators
        if(*p == '<') {
            *p++ = '\0';  // Terminate current argument
            while(*p && isspace((unsigned char)*p))
                p++;
            char *start = p;
            // Find end of filename
            while(*p && !isspace((unsigned char)*p) && *p != '<' && *p != '>')
                p++;
            if(*p) {
                char temp = *p;
                *p = '\0';
                cmd_buff->input_file = strdup(trim_whitespace(start));
                *p = temp;
            } else {
                cmd_buff->input_file = strdup(trim_whitespace(start));
            }
            continue;
        } 
        else if(*p == '>') {
            *p++ = '\0';  // Terminate current argument
            
            // Check for append mode (>>)
            if(*p == '>') {
                cmd_buff->append_mode = 1;
                p++;
            }
            
            while(*p && isspace((unsigned char)*p))
                p++;
            char *start = p;
            // Find end of filename
            while(*p && !isspace((unsigned char)*p) && *p != '<' && *p != '>')
                p++;
            if(*p) {
                char temp = *p;
                *p = '\0';
                cmd_buff->output_file = strdup(trim_whitespace(start));
                *p = temp;
            } else {
                cmd_buff->output_file = strdup(trim_whitespace(start));
            }
            continue;
        }
        
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
            while(*p && !isspace((unsigned char)*p) && *p != '<' && *p != '>')
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
 
int build_cmd_list(char *cmd_line, command_list_t *clist) {
    char *token;
    char *saveptr;
    int cmd_count = 0;
    
    // Make a copy of cmd_line to tokenize
    char *line_copy = strdup(cmd_line);
    if (!line_copy) {
        return ERR_MEMORY;
    }
    
    // Initialize command list
    clist->num = 0;
    
    // First, count the number of pipe segments to validate against CMD_MAX
    char *count_copy = strdup(cmd_line);
    if (!count_copy) {
        free(line_copy);
        return ERR_MEMORY;
    }
    
    char *count_token;
    char *count_saveptr;
    int pipe_count = 0;
    
    count_token = strtok_r(count_copy, PIPE_STRING, &count_saveptr);
    while (count_token != NULL) {
        pipe_count++;
        count_token = strtok_r(NULL, PIPE_STRING, &count_saveptr);
    }
    
    free(count_copy);
    
    // Check if pipe count exceeds CMD_MAX
    if (pipe_count > CMD_MAX) {
        fprintf(stderr, CMD_ERR_PIPE_LIMIT, CMD_MAX);
        free(line_copy);
        return ERR_TOO_MANY_COMMANDS;
    }
    
    // Tokenize by pipe character for actual processing
    token = strtok_r(line_copy, PIPE_STRING, &saveptr);
    while (token != NULL && cmd_count < CMD_MAX) {
        char *trimmed = trim_whitespace(token);
        if (strlen(trimmed) > 0) {
            // Allocate buffer for the command
            if (alloc_cmd_buff(&clist->commands[cmd_count]) != OK) {
                free(line_copy);
                return ERR_MEMORY;
            }
            
            // Build command from the token
            if (build_cmd_buff(trimmed, &clist->commands[cmd_count]) == OK) {
                cmd_count++;
            }
        }
        token = strtok_r(NULL, PIPE_STRING, &saveptr);
    }
    
    free(line_copy);
    
    clist->num = cmd_count;
    return (cmd_count > 0) ? OK : WARN_NO_CMDS;
}

int free_cmd_list(command_list_t *cmd_list) {
    for (int i = 0; i < cmd_list->num; i++) {
        free_cmd_buff(&cmd_list->commands[i]);
    }
    cmd_list->num = 0;
    return OK;
}

int execute_pipeline(command_list_t *clist) {
    if (clist->num == 0) {
        return WARN_NO_CMDS;
    }
    
    // If only one command, check if it's a built-in
    if (clist->num == 1) {
        cmd_buff_t *cmd = &clist->commands[0];
        Built_In_Cmds bi = match_command(cmd->argv[0]);
        if (bi != BI_NOT_BI) {
            // Built-in commands don't support redirection
            if (cmd->input_file != NULL || cmd->output_file != NULL) {
                fprintf(stderr, "Redirection not supported for built-in commands\n");
                return ERR_EXEC_CMD;
            }
            exec_built_in_cmd(cmd);
            return OK;
        }
    }
    
    int pipes[CMD_MAX-1][2]; // Array of pipes
    pid_t pids[CMD_MAX];     // Array to store child PIDs
    
    // Create all necessary pipes
    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            return ERR_EXEC_CMD;
        }
    }
    
    // Fork and execute each command in the pipeline
    for (int i = 0; i < clist->num; i++) {
        cmd_buff_t *cmd = &clist->commands[i];
        
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            return ERR_EXEC_CMD;
        }
        
        if (pids[i] == 0) {
            // Child process
            
            // Set up input redirection (except for the first command)
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            // Set up output redirection (except for the last command)
            if (i < clist->num - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            // Close all pipe file descriptors in the child
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Handle file redirection
            int fd;
            
            // Input redirection (<)
            if (cmd->input_file != NULL) {
                fd = open(cmd->input_file, O_RDONLY);
                if (fd < 0) {
                    fprintf(stderr, "Cannot open input file: %s\n", cmd->input_file);
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            
            // Output redirection (> or >>)
            if (cmd->output_file != NULL) {
                int flags = O_WRONLY | O_CREAT;
                
                if (cmd->append_mode) {
                    flags |= O_APPEND;  // For >>
                } else {
                    flags |= O_TRUNC;   // For >
                }
                
                fd = open(cmd->output_file, flags, 0644);
                if (fd < 0) {
                    fprintf(stderr, "Cannot open output file: %s\n", cmd->output_file);
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            
            // Execute the command
            execvp(cmd->argv[0], cmd->argv);
            
            // If we get here, execvp failed
            if (errno == ENOENT) {
                fprintf(stderr, "Command not found in PATH\n");
            } else if (errno == EACCES) {
                fprintf(stderr, "Permission denied\n");
            } else {
                fprintf(stderr, "execvp error: %s\n", strerror(errno));
            }
            exit(errno);
        }
    }
    
    // Parent process: close all pipe file descriptors
    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Wait for all children to complete
    int status;
    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], &status, 0);
        if (i == clist->num - 1) { // Only store exit status of last command in pipeline
            if (WIFEXITED(status)) {
                last_rc = WEXITSTATUS(status);
            } else {
                last_rc = -1;
            }
        }
    }
    
    return OK;
}

int exec_local_cmd_loop() {
    char input_line[SH_CMD_MAX + 1];
    command_list_t cmd_list;
    
    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(input_line, sizeof(input_line), stdin) == NULL) {
            printf("\n");
            break;
        }
        
        // Remove trailing newline
        input_line[strcspn(input_line, "\n")] = '\0';
        
        // Skip empty lines
        if (strlen(trim_whitespace(input_line)) == 0) {
            continue;
        }
        
        // Parse the command line into a command list
        int rc = build_cmd_list(input_line, &cmd_list);
        
        if (rc == WARN_NO_CMDS) {
            fprintf(stderr, CMD_WARN_NO_CMD);
            continue;
        } else if (rc == ERR_TOO_MANY_COMMANDS) {
            fprintf(stderr, CMD_ERR_PIPE_LIMIT, CMD_MAX);
            continue;
        } else if (rc != OK) {
            continue;
        }
        
        // Execute the command pipeline
        execute_pipeline(&cmd_list);
        
        // Free the command list resources
        free_cmd_list(&cmd_list);
    }
    
    return OK;
}