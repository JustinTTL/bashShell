/*
*	shell.c
*   
*	A bash-like shell with I/O redirection, program invocation, custom 
*	environments and standard functions and concurrency options.
*
*	Contains main for shell program, and uses functions located in
*	shell_func.h for the general shell operations, and methods in 
*	shell_util.h for shell utilities and helper functions.
*   
*   by 
*     	Justin Tze Tsun Lee
*     	Matthew Carrington-Fair
*     	Tomer Shapira
*
*
*/

/* Declaration File and Parsing Environment Helper Files */
#include "shell_func.h"
#include "shell_util.h"


int main(int argc, char *argv[]) {
	
	/* Sets up environment */
	set_environ_variables(argv[0]);

	/* Checks arguments*/
	FILE *fp;
	if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}
	/* Assigns output to either stdin or batch file*/
	else if (argc == 1)
		fp = stdin;
	else {
		fp = fopen(argv[1], "r");
		if (fp == NULL){
			fprintf(stderr, "Error opening file\n");
			exit(EXIT_FAILURE);
		}
	}
	/* Begins main loop */
	run_shell(fp);
	return(EXIT_SUCCESS);
}