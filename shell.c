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

/* Function Declaration */
void run_shell();
void process_instructions(char *prompt);
void execute_instructions(char *instructions[]);
char **separate_string(char *string, char *delim);
void launch(char *instructions[], pid_t *child_pid);

/* Operations Enumeration and lookup function */
enum ops {CD, CLR, DIR, ENVIRON, ECHO, HELP, PAUSE, QUIT, ITEM_NONE};
const char* lookup_table[] = { "cd", "clr", "dir", "environ", "echo", "help", "pause", "quit" };

int lookup(char* op) {
    const int available_ops = sizeof lookup_table / sizeof *lookup_table;
    for(int i=0; i != available_ops; i++)
        if (strcmp(op, lookup_table[i]) == 0)
            return i;
    return ITEM_NONE;
}

/* Calls Shell infinite loop */
int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	run_shell(); 
	return EXIT_SUCCESS;
}

/* Main loop to read in new lines of input */
void run_shell() {
	while (1) {
		char *raw_prompt = NULL;
		size_t size = 0;
		printf("&> ");
		int input_size = (int)getline(&raw_prompt, &size, stdin);

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
		char **args = separate_string(instruction_list[i], " ");
		execute_instructions(args);
		free(args);
	}

	free(instruction_list);
	return;
}


/* Execute Individual Operations */
void execute_instructions(char *instructions[]) {
	char *instruction = instructions[0];
	pid_t child_pid = 0;
	if (strcmp(instruction, "quit") == 0) {
		exit(EXIT_SUCCESS);
	}
	if (lookup(instruction) != ITEM_NONE){
		launch(instructions, &child_pid);
	}
	else {
		printf("%s: Command not found.\n", instruction);
	}
		
	if (child_pid > 0){
		waitpid(child_pid, NULL, 0);
	}
	return;
}

void launch(char *instructions[], pid_t *child_pid) {
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
		else 
			execve(instructions[0], instructions, envp);
		
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
			size = size*2;
			string_list = realloc(string_list, size * sizeof(*string_list));
			assert(string_list != NULL);
		}

		indiv_string = strtok(NULL, delim);
	}

	string_list[i] = NULL;

	return string_list;
}
