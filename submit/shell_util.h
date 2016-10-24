/*
*       shell_util.h
*
*       This file contains utilities and helper functions for 
*       shell.c
*
*       It initializes the state of the shell, initializes the state of any new
*       processes that are launched, and provides helper methods in separating 
*       instructions and arguments.
*   
*/

/* Libraries */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <termcap.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>

/* Constant Flags */
#define BUFFER_SIZE 64
#define NOT_FOUND -1
#define SUCCESS 1
#define ERROR -1
#define BCKGRND 0
#define FRGRND 1


/* Function Declaration */

/* Inits environment variables for shell */
void set_environ_variables(char *executable);
/* Creates environment arguments for child processes */
void make_env(char **envp[]);
/* Separates a string based on passed deliminator into list*/
char **separate_string(char *string, char *delim);
/* Returns size of list */
int str_list_length(char **str_list);
