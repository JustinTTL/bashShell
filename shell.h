/*
*	shell.h
*   
*	A bash-like shell with I/O redirection, program invocation, custom 
*	environments and standard functions and concurrency options.
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

/* Holds instruction args and FP for output */
typedef struct args_io_struct{
	char **instruction_list;
	FILE *output_file;
	int wait_status;
}args_io_struct;


/* Function Declaration */
void run_shell(FILE *fp);

void process_instructions(char *prompt);
void *handle_instruction(void *instruction_list);
int handle_pipe_background(args_io_struct *instruction_io);

void execute_instructions(args_io_struct instruction_io);

void launch(args_io_struct instruction_io, pid_t *child_pid);

void background_handler(int signo);

void quit_shell();


