/*
*       shell_util.c
*   
*       Utility functions for shell.c
*
*       Contains the methods:
*
*               set_environ_variables(char * executable)
*               make_env(char **envp[])
*               separate_string(char *string, char *delim)
*               str_list_length(char **str_list)
*   
*   by 
*       Justin Tze Tsun Lee
*       Matthew Carrington-Fair
*       Tomer Shapira
*
*/

#include "shell_util.h"


/* Utility function to initilize environment vars */
void set_environ_variables(char *executable) {
        /* First set shell environment variable */
        char buffer[PATH_MAX];
    realpath(executable, buffer);
        setenv("shell", buffer, 1);

        /* Then, construct path to directory of executables */
        char executable_path[PATH_MAX] = {"\0"};
        char **dir = separate_string(buffer, "/");
        for (int i = 0; dir != NULL; i++) {
                /* Stops at second to last segment which would
                simply contain shell executable name */
                if (dir[i + 1] == NULL) {
                        dir[i] = NULL;
                        break;
                }
                else {
                        strcat(executable_path, "/");
                        strcat(executable_path, dir[i]);
                }
        }
        free(dir);
        strcat(executable_path, "/");
        /* Sets it to EXEC var */
        setenv("EXEC", executable_path, 1);
}
/* Constructs a list of environment variables to pass when launching child */
void make_env(char **envp[]){

        /* Creates PATH, EXEC, parent, and shell vars */
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

        /* Appends variable to end of key-value assignment */
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
/* Small helper function to retrieve length of a list */
int str_list_length(char **str_list){
        int len = 0;
        while (str_list[len] != NULL){
                len++;
        }
        return len;
}

