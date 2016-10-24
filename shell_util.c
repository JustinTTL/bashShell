/*
*	shell_util.c
*   
*/

#include "shell_util.h"

void set_environ_variables(char *executable) {
	/* First set shell environment variable */
	char buffer[PATH_MAX];
    realpath(executable, buffer);
	setenv("shell", buffer, 1);
	char executable_path[PATH_MAX];
	char **dir = separate_string(buffer, "/");
	for (int i = 0; dir != NULL; i++) {
		if (dir[i + 1] == NULL) {
			dir[i] = NULL;
			break;
		}
		else {
			strcat(executable_path, "/");
			strcat(executable_path, dir[i]);
		}
	}
	printf("dir: %s\n", executable_path);

	strcat(executable_path, "/");
	setenv("EXEC", executable_path, 1);
}

void make_env(char **envp[]){
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

	char *environ [] = {parent_buffer, path_buffer, exec_buffer, shell_buffer, NULL};
	*envp = environ;
	return;
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

int str_list_length(char **str_list){
	int len = 0;
	while (str_list[len] != NULL){
		len++;
	}
	return len;
}

