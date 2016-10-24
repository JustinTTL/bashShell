/*
*	shell_util.h
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
void set_environ_variables(char *executable);
void make_env(char **envp[]);
char **separate_string(char *string, char *delim);
int str_list_length(char **str_list);
