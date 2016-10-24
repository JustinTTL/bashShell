#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>


int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    char *exec_path = getenv("EXEC");
    char *file_name = "manual.txt";
    char *more_cmd = "more -df ";

    size_t cmd_buf_len = strlen(exec_path) + strlen(file_name) + strlen(more_cmd);
    char cmd_buf[cmd_buf_len];

    strcat(cmd_buf, more_cmd);
   	strcat(cmd_buf, exec_path);
   	strcat(cmd_buf, file_name);

    system(cmd_buf);
    return EXIT_SUCCESS;
} 