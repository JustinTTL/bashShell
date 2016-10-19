/*
*	shell.c
*
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <termcap.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>

#define BUFFER_SIZE 64
#define NOT_FOUND -1
#define SUCCESS 1
#define ERROR -1

typedef struct args_io_struct{
	char **instruction_list;
	FILE *output_file;
}args_io_struct;

/* Function Declaration */
void run_shell(FILE *fp);
void process_instructions(char *prompt);
void execute_instructions(args_io_struct instruction_io);
int handle_pipe(char *argu_v[], args_io_struct *instruction_io);
char **separate_string(char *string, char *delim);
void launch(args_io_struct instruction_io, pid_t *child_pid);
char **re_size(char *string_list[], int *size);

/* Operations Enumeration and lookup function */
enum ops {CD, CLR, DIR, ENVIRON, ECHO, HELP, PAUSE, QUIT, ITEM_NONE};
const char* lookup_table[] = { "cd", "clr", "dir", "environ", "echo", "help", "pause", "quit" };

int lookup(char* op) {
    const int available_ops = sizeof lookup_table / sizeof *lookup_table;
    for(int i=0; i != available_ops; i++)
        if (strcmp(op, lookup_table[i]) == 0)
            return i;
    return NOT_FOUND;
}


/* Calls Shell infinite loop */
int main(int argc, char *argv[]) {
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

void run_shell(FILE *fp) {
	while (1) {
		char *raw_prompt = NULL;
		size_t size = 0;
		int input_size = 0;
		if (fp == stdin)
			printf("%s > ", getenv("PWD"));
		input_size = (int)getline(&raw_prompt, &size, fp);	

		/* EOF Handling */
		if(input_size == -1){
			printf("\n");
			free(raw_prompt);
			exit(EXIT_SUCCESS);
		}

		/* Removing the newline character at the end of getline */
		char *prompt = strtok(raw_prompt, "\n");

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
		int pipe_status = handle_pipe(argu_v, &instruction_io);


		if (pipe_status == ERROR){
			free(argu_v);
		} 
		else if (argu_v[0] != NULL) {
			execute_instructions(instruction_io);
			free(argu_v);
			if (instruction_io.output_file != stdin){
				fclose(instruction_io.output_file);
			}
		}
	}

	free(instruction_list);
	return;
}


int handle_pipe(char *argu_v[], args_io_struct *instruction_io) {
	int pipe_loc = NOT_FOUND;
	int i;
	char *io_mode;

	for (i = 0; argu_v[i] != NULL; i++) {
		/* Overwrite Mode */
		if (strcmp(argu_v[i], ">") == 0){
			pipe_loc = i;
			io_mode = "w";
		}
		/* Appending Mode */
		if (strcmp(argu_v[i], ">>") == 0){
			pipe_loc = i;
			io_mode = "a";
		}
	}

	if(pipe_loc == NOT_FOUND){
		instruction_io -> instruction_list = argu_v;
		instruction_io -> output_file = stdin;
		return SUCCESS;
	}
	else {
		int num_argu = (i - 1) - pipe_loc;
		if (num_argu == 1){
			FILE *out_file = fopen(argu_v[pipe_loc+1], io_mode);
			if (out_file == NULL) {
				printf("Error: Cannot open file.\n");
				return ERROR;
			}
			else {
				argu_v[pipe_loc] = NULL;
				instruction_io -> instruction_list = argu_v;
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
		exit(EXIT_SUCCESS);
	}
	else if (strcmp(instruction, "cd") == 0) {
		exit(EXIT_SUCCESS);
	}
	else if (lookup(instruction) != NOT_FOUND){
		launch(instruction_io, &child_pid);
	}
	else {
		printf("%s: Command not found.\n", instruction);
	}
	
	/* Wait for Child Process */	
	if (child_pid > 0){
		waitpid(child_pid, NULL, 0);
	}

	return;
}

void launch(args_io_struct instruction_io, pid_t *child_pid) {
	char *envp [] = {NULL};

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
			execve(instruction_io.instruction_list[0], instruction_io.instruction_list, envp);
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