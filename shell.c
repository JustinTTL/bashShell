/*
*	shell.c
*
*
*
*/

/* Libraries */
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

/* Constant Flags */
#define BUFFER_SIZE 64
#define NOT_FOUND -1
#define SUCCESS 1
#define ERROR -1
#define BCKGRND 0
#define FRGRND 1

/* Global flag */
static int RETURN_PROCESS = NOT_FOUND;

/* Holds instruction args and FP for output */
typedef struct args_io_struct{
	char **instruction_list;
	FILE *output_file;
	int wait_status;
}args_io_struct;

/* Function Declaration */
void set_environ_variables(char *executable);
void run_shell(FILE *fp);
void process_instructions(char *prompt);
void execute_instructions(args_io_struct instruction_io);
int handle_pipe_background(args_io_struct *instruction_io);
char **separate_string(char *string, char *delim);
void launch(args_io_struct instruction_io, pid_t *child_pid);
char **re_size(char *string_list[], int *size);
void background_handler(int signo);
void quit_shell();

/* Handles any children that are background processes */
void background_handler(int signo) {
	(void)signo;
	pid_t pid;

	/* Terminate the Zombie Child Process */
	pid = wait(NULL);
	/* Update global flag with pid so next iteration of the 
	 * shell will show the terminiation of the background process 
	 */
	if (pid > 0) {
		RETURN_PROCESS = pid;
	}	
}

int main(int argc, char *argv[]) {
	
	/* Sets up environment */
	set_environ_variables(argv[0]);

	FILE *fp;
	if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}
	else if (argc == 1)
		fp = stdin;
	else {
		fp = fopen(argv[1], "r");
		if (fp == NULL){
			fprintf(stderr, "Error opening file\n");
			exit(EXIT_FAILURE);
		}
	}
	run_shell(fp);
	return(EXIT_SUCCESS);
}

void set_environ_variables(char *executable) {
	/* First set shell environment variable */
	char buffer[PATH_MAX];
    realpath(executable, buffer);
	setenv("shell", buffer, 1);

	/* Then set path to the directory of the shell executable 
	by first getting current directory, and adding given executable 
	(in argv) to get path back to original directory */
	getcwd(buffer, sizeof(buffer));
	strcat(buffer, "/");	
	/* Add directories one by one until at directory for executable */
	char **executable_path = separate_string(executable, "/");
	for (int i = 0; executable_path[i] != NULL; i++) {
		if (executable_path[i + 1] != NULL) {
			strcat(buffer, executable_path[i]);
			strcat(buffer, "/");

		}
	}
	/* If executable was called in its native directory */
	strtok(buffer, ".");
	setenv("EXEC", buffer, 1);
}
/* Main shell process */
void run_shell(FILE *fp) {
	/* Setup signal handler for child processes */
	signal(SIGCHLD, background_handler);
	while (1) {
		char *raw_prompt = NULL;
		size_t size = 0;
		int input_size = 0;

		/* If there is a background process that has finished */
		if (RETURN_PROCESS != NOT_FOUND) {
			printf("[1]+ Done\n");
			/* Reset flag */
			RETURN_PROCESS = NOT_FOUND;
		}
		/* Interactive Mode */
		if (fp == stdin) {
			char buffer[200];
			printf("(myshell) %s > ", getcwd(buffer, sizeof(buffer)));
		}

		input_size = (int)getline(&raw_prompt, &size, fp);

		/* EOF Handling */
		if(input_size == -1){
			printf("\n");
			free(raw_prompt);
			quit_shell();
		}

		/* Removing the newline character at the end of getline */
		char *prompt = strtok(raw_prompt, "\n");

		/* Batch Mode Input Echoing*/
		if (fp != stdin){
			printf("%s\n",  prompt);
		}

		/* Pull out instructions in input */
		process_instructions(prompt);
		free(raw_prompt);
	} 
}

/* Executes instructions in a line */
void process_instructions(char *prompt) {
	char **instruction_list = separate_string(prompt, ";");

	/* Second iteration to seperate individual arguments within instruction */
	for (int i = 0; instruction_list[i] != NULL; i++) {
		char **argu_v = separate_string(instruction_list[i], " ");

		args_io_struct instruction_io;
		instruction_io.instruction_list = argu_v;
		instruction_io.output_file = stdout;
		instruction_io.wait_status = FRGRND;

		int status = handle_pipe_background(&instruction_io);

		if (status == ERROR){
			free(argu_v);
		} 
		else if (argu_v[0] != NULL) {
			execute_instructions(instruction_io);
			free(argu_v);
			if (instruction_io.output_file != stdout){
				fclose(instruction_io.output_file);
			}
		}
	}

	free(instruction_list);
	return;
}

/* Determines whether instructon pipes to a file or runs in background*/
int handle_pipe_background(args_io_struct *instruction_io) {
	int pipe_loc = NOT_FOUND;
	int i;
	char *io_mode;

	for (i = 0; instruction_io -> instruction_list[i] != NULL; i++) {
		if (strcmp(instruction_io -> instruction_list[i], "&") == 0){
			if (instruction_io -> instruction_list[i+1] == NULL) {
				instruction_io -> wait_status = BCKGRND;
				instruction_io -> instruction_list[i] = NULL;
			}
			else {
				printf("Error: Misplaced & token\n");
				return ERROR;
			}

		}
	}

	for (i = 0; instruction_io -> instruction_list[i] != NULL; i++) {
		/* Overwrite Mode */
		if (strcmp(instruction_io -> instruction_list[i], ">") == 0){
			pipe_loc = i;
			io_mode = "w";
		}
		/* Appending Mode */
		if (strcmp(instruction_io -> instruction_list[i], ">>") == 0){
			pipe_loc = i;
			io_mode = "a";
		}
	}

	if(pipe_loc == NOT_FOUND){
		return SUCCESS;
	}
	else {
		int num_argu = (i - 1) - pipe_loc;
		if (num_argu == 1){
			FILE *out_file = fopen(instruction_io -> instruction_list[pipe_loc+1], io_mode);
			if (out_file == NULL) {
				printf("Error: Cannot open file.\n");
				return ERROR;
			}
			else {
				instruction_io -> instruction_list[pipe_loc] = NULL;
				instruction_io -> output_file = out_file;
				return SUCCESS;
			}
		}
		else if (num_argu == 0) {
			printf("Error: No pipeline arguements.\n");
			return ERROR;
		}
		else {
			printf("Error: Too many pipeline arguements.\n");
			return ERROR;
		}
	}
}


/* Execute Individual Operations */
void execute_instructions(args_io_struct instruction_io) {
	char *instruction = instruction_io.instruction_list[0];
	pid_t child_pid = 0;

	if (strcmp(instruction, "quit") == 0) {
		quit_shell();
	}
	else if (strcmp(instruction, "cd") == 0) {
		if (chdir(instruction_io.instruction_list[1]) == NOT_FOUND) {
			fprintf(stderr, "%s: No such file or directory\n", instruction_io.instruction_list[1]);
		}
		else {
		char buffer[PATH_MAX];
		setenv("PWD", getcwd(buffer, sizeof(buffer)), 1);
		}
	}
	//else if (lookup(instruction) != NOT_FOUND){
	else {
		launch(instruction_io, &child_pid);
	}
	

	/* Wait for Child Process */	
	if (child_pid > 0 && instruction_io.wait_status == FRGRND){
		waitpid(child_pid, NULL, 0);
	}
	if (instruction_io.wait_status == BCKGRND) {
		printf("[1] %d\n", child_pid);
	}

	return;
}

void launch(args_io_struct instruction_io, pid_t *child_pid) {


	char path_buffer[PATH_MAX];
	char exec_buffer[PATH_MAX];
	char parent_buffer[PATH_MAX];
	char shell_buffer[PATH_MAX];

	strcpy(path_buffer, "PATH=");
	strcpy(exec_buffer, "EXEC=");
	strcpy(parent_buffer, "parent=");
	strcpy(shell_buffer, "shell=");

	char *path = getenv("PATH");
	char *exec = getenv("EXEC");
	char *shell = getenv("shell");

	strcat(path_buffer, path);
	strcat(exec_buffer, exec);
	strcat(parent_buffer, shell);
	strcat(shell_buffer, shell);


	char *envp [] = {parent_buffer, path_buffer, exec_buffer, NULL};
	char command[PATH_MAX];
	strcpy(command, exec);
	strcat(command, "/");
	strcat(command, instruction_io.instruction_list[0]);
	/* Forking */
	*child_pid = fork();
		/* Error Checking or Parent Breaking*/
		if(*child_pid != 0){  
			if(*child_pid < 0)
				fprintf(stderr, "Forking Error\n");
			return;
		}
		/* Child Process */
		else{
			dup2(fileno(instruction_io.output_file), 1);
			if (execve(instruction_io.instruction_list[0], instruction_io.instruction_list, envp) == NOT_FOUND) {
				if (execve(command, instruction_io.instruction_list, envp) == NOT_FOUND) {
					fprintf(stderr, "%s: command not found\n", instruction_io.instruction_list[0]);
					exit(EXIT_FAILURE);
				}
			}
		}
		
}
/* Takes string and seperates by delim into several strings */
char **separate_string(char *string, char *delim) {

	/* Init the size */
	int size = BUFFER_SIZE;
	char **string_list = malloc(BUFFER_SIZE * sizeof(*string_list));
	assert(string_list != NULL);

	/* Starts loop to pull out strings until NULL */
	char *indiv_string = strtok(string, delim);
	int i = 0;
	while (indiv_string != NULL) {
		string_list[i] = indiv_string;
		i++;

		/* Resizes if reaches buffer limit */
		if (i == size) {
			size = size * 2;
			string_list = realloc(string_list, size * sizeof(*string_list));
			assert(string_list != NULL);
		}
		indiv_string = strtok(NULL, delim);
	}
	string_list[i] = NULL;
	return string_list;
}

/* Safe shell quitting */
void quit_shell(){
	signal(SIGCHLD, SIG_DFL);

	/* Properly begin exit sequence by waiting for ALL children */
	pid_t child_pid;

	printf("Begin Exit Sequence: Waiting for children termination\n");

	while ((child_pid = wait(NULL))){
		//child_pid = wait(NULL);
		if (errno == ECHILD){
			break;
		}
		fprintf(stderr,		"%d terminated\n", (int)child_pid);
	}

	printf("Exitting\n");
	exit(EXIT_SUCCESS);
}