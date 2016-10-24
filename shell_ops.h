/*
*	shell.h
*   
*	A bash-like shell with I/O redirection, program invocation, custom 
*	environments and standard functions and concurrency options.
*
*	This file contains the major functions that handle the operation of the
*	shell including parsing instructions, processing them for I/O redirection
*	and running in background, and executing instructions by launching them
*	as child processes. 
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
#include "shell_util.h"


/* CODE Flags */
#define BUFFER_SIZE 64
#define NOT_FOUND -1
#define SUCCESS 1
#define ERROR -1
#define BCKGRND 0
#define FRGRND 1


/* Concurrency Flag */
#define CONCURR 0


/* Holds instruction args and FP for output */
typedef struct args_io_struct{
	char **instruction_list;
	FILE *output_file;
	int wait_status;
}args_io_struct;


/* Function Declaration */

/* Main loop for running shell*/
void run_shell(FILE *fp);
/* Determines whether to run instructions concurrently*/
void process_instructions(char *prompt);
/* Vets instructions for any special I/O redirection or background processing*/
void *handle_instruction(void *instruction_list);
/* Sets flags for I/O redirection or background processing*/
int handle_pipe_background(args_io_struct *instruction_io);
/* Executes given instruction set*/
void execute_instructions(args_io_struct instruction_io);
/* Launches new process */
void launch(args_io_struct instruction_io, pid_t *child_pid);
/* Handles signals from child processes running in background */
void background_handler(int signo);
/* Safely quits the shell */
void quit_shell();

