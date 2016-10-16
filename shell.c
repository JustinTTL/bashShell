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

void run_shell();
void process_instructions(char *prompt);
void execute_instructions(char **instructions);
char **separate_string(char *string, char *delim);


int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	run_shell(); 
	return EXIT_SUCCESS;
}
/* Main loop to read in new lines of input */
void run_shell() {
	while (1) {
		/* TODO EOF stuff */
		char *prompt = NULL;
		size_t size = 0;
		printf("&> ");
		int input_size = (int)getline(&prompt, &size, stdin);
		if(input_size == -1){
			printf("\n");
			exit(EXIT_SUCCESS);
		}

		prompt = strtok(prompt, "\n");

		/* Pull out instructions in input */
		process_instructions(prompt);
		free(prompt);
	}
}
/* Returns array of strings for each word in input */
void process_instructions(char *prompt) {
	
	char **instruction_list = separate_string(prompt, ";");
	/* Second iteration to seperate individual arguments within instruction */
	int i = 0;
	while (instruction_list[i] != NULL) {
		char **args = separate_string(instruction_list[i], " ");
		execute_instructions(args);
		i++;
	}
	free(instruction_list);
}

void execute_instructions(char **instructions) {
	if (instructions[0] == NULL) {
		return;
	}
	int i = 0;
	while (instructions[i] != NULL) {
		if (strcmp(instructions[i], "echo") == 0) {
			i++;
			while(instructions[i] != NULL) {
				printf("%s ", instructions[i]);
				i++;
			}
			printf("\n");
			free(instructions);
			return;
		}
		else if (strcmp(instructions[i], "clr") == 0) {
			printf("\033[H\033[J");
		}
		else if (strcmp(instructions[i], "pause") == 0) {
			system("read");		
		}
		else if (strcmp(instructions[i], "help") == 0) {
		}
		else if (strcmp(instructions[i], "quit") == 0)	 {
			exit(EXIT_SUCCESS);
		}
		else if (strcmp(instructions[i], "\n") == 0) {
			return;
		}
		else {
			printf("%s: Command not found.\n", instructions[i]);
			return;
		}	
		i++;
		free(instructions[i]);
	}
}
/* Takes string and seperates by delim into several strings */
char **separate_string(char *string, char *delim) {
	int i = 0;
	/* Init the size */
	int size = BUFFER_SIZE;
	char **string_list = malloc(BUFFER_SIZE * sizeof(*string_list));
	assert(string_list);
	/* Starts loop to pull out strings until NULL */
	char *indiv_string = strtok(string, delim);
	while (indiv_string != NULL) {
		string_list[i] = indiv_string;
		i++;
		/* Resizes if reaches buffer limit */
		if (i == size) {
			size += BUFFER_SIZE;
			string_list = realloc(string_list, size * sizeof(*string_list));
			assert(string_list);
		}
		indiv_string = strtok(NULL, delim);
	}
	string_list[i] = NULL;
	return string_list;
}