#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */

 static char *trim_whitespace(char *str)
{
    char *end;
    // trim leading space
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0)  // all spaces?
        return str;
    // trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    // write new null terminator
    *(end+1) = '\0';
    return str;
}

int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    // initialize command list count
    clist->num = 0;

    // trim overall input
    char *trimmed_line = trim_whitespace(cmd_line);
    if(strlen(trimmed_line) == 0) {
        return WARN_NO_CMDS;
    }

    // tokenize input by PIPE_CHAR
    char *saveptr = NULL;
    char *token = strtok_r(trimmed_line, PIPE_STRING, &saveptr);
    while(token != NULL) {
        if (clist->num >= CMD_MAX)
            return ERR_TOO_MANY_COMMANDS;

        // trim spaces in the command substring
        char *cmd_sub = trim_whitespace(token);
        if(strlen(cmd_sub) == 0) {
            // if the command is empty  treat it as no command provided
            token = strtok_r(NULL, PIPE_STRING, &saveptr);
            continue;
        }

        // get the index for current command
        int idx = clist->num;

        // parse the command: first token is executable, rest are arguments
        char *arg_saveptr = NULL;
        char *exe_token = strtok_r(cmd_sub, " \t", &arg_saveptr);
        if(exe_token == NULL) {
            token = strtok_r(NULL, PIPE_STRING, &saveptr);
            continue;
        }
        if(strlen(exe_token) >= EXE_MAX)
            return ERR_CMD_OR_ARGS_TOO_BIG;
        strcpy(clist->commands[idx].exe, exe_token);

        // prepare to build the args string
        clist->commands[idx].args[0] = '\0';
        int first_arg = 1;
        char *arg_token = strtok_r(NULL, " \t", &arg_saveptr);
        while(arg_token != NULL) {
            if(!first_arg)
                strcat(clist->commands[idx].args, " ");
            first_arg = 0;
            // check length for arguments
            if((strlen(clist->commands[idx].args) + strlen(arg_token) + 1) >= ARG_MAX)
                return ERR_CMD_OR_ARGS_TOO_BIG;
            strcat(clist->commands[idx].args, arg_token);
            arg_token = strtok_r(NULL, " \t", &arg_saveptr);
        }
        clist->num++;
        token = strtok_r(NULL, PIPE_STRING, &saveptr);
    }
    return OK;
}