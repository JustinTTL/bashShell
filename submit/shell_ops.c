/*
*       shell_func.c
*   
*       Core fnctions for shell.c
*
*       Contains the methods:
*
*       void run_shell(FILE *fp);
*       void process_instructions(char *prompt);
*       void *handle_instruction(void *instruction_list);
*       int handle_pipe_background(args_io_struct *instruction_io);
*       void execute_instructions(args_io_struct instruction_io);
*       void launch(args_io_struct instruction_io, pid_t *child_pid);
*       void background_handler(int signo);
*       void quit_shell();
*   
*   by 
*       Justin Tze Tsun Lee
*       Matthew Carrington-Fair
*       Tomer Shapira
*
*/

#include "shell_ops.h"

/* Global flag for terminated child signal handler*/
static int RETURN_PROCESS = NOT_FOUND;

/* Main shell process */
void run_shell(FILE *fp) {
        /* Setup signal handler for child processes 
        only if not running in concurrent mode */
        if (!CONCURR){
                signal(SIGCHLD, background_handler);
        }

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
                /* Receives line of user input */
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
                /* Makes copy of input so we can free memory allocated
                by getline */
                int len = 0;
                if (prompt != NULL) {
                        len = strlen(prompt);
                        char prompt_copy[len];
                        int i;
                        for (i = 0; i < len; i++) {
                                prompt_copy[i] = raw_prompt[i];
                        }
                        prompt_copy[i] = '\0';
                        free(raw_prompt);
                        /* Pull out instructions in input */
                        process_instructions(prompt_copy);
                } 
                /* If user simply hits enter */
                else {
                        free(raw_prompt);
                }
        } 
}

/* Processes instructions depending on whether shell is 
running in concurrent mode or not */
void process_instructions(char *prompt) {
        /* Separates instructions */
        char **instruction_list = separate_string(prompt, ";");
        /* If running concurrently, launches each instruction in separate thread*/
        if (CONCURR){
                int instruction_num = str_list_length(instruction_list);
                pthread_t threads[instruction_num];

                /* Creates and checks threads */
                for (int i = 0; instruction_list[i] != NULL; i++) {
                        if (pthread_create(&threads[i],
                                NULL,
                                handle_instruction,
                                (void *)instruction_list[i])) {
                                fprintf(stderr, "Error making threads\n");
                                free(instruction_list);
                                return;
                        }
                }
                /* Waits for all threads of a single instruction set to return
                before proceeding */
                for (int i = 0; i < instruction_num; i++) {
                        pthread_join(threads[i], NULL);
                }
        }
        /* If not concurrent */
        else {
                for (int i = 0; instruction_list[i] != NULL; i++) {
                        /* Checks if instruction is a quit */
                        char copy[PATH_MAX];
                        strcpy(copy, instruction_list[i]);
                        if (strcmp(strtok(copy, "\t "), "quit") == 0 ) {
                                free(instruction_list);
                                quit_shell();
                        }
                        /* If not, proceeds */
                        handle_instruction((void *)instruction_list[i]);
                }
                free(instruction_list);
        }
        return;
}
/* Performs pre-execution vetting of instructions to determine 
if there are any special conditions */
void *handle_instruction(void *instruction_list) {
        char *instruction = (char*)instruction_list;
        /* Breaks up instruction into its arguments */
        char **argu_v = separate_string(instruction, "\t ");

        /* assigns arguments, out_put file and wait_status */
        args_io_struct instruction_io;
        instruction_io.instruction_list = argu_v;
        instruction_io.output_file = stdout;
        instruction_io.wait_status = FRGRND;

        int status = handle_pipe_background(&instruction_io);

        /* If there is any error parsing for piping or &*/
        if (status == ERROR){
                free(argu_v);
        } 
        /* If no error */
        else if (argu_v[0] != NULL) {
                execute_instructions(instruction_io);
                free(argu_v);
                if (instruction_io.output_file != stdout){
                        fclose(instruction_io.output_file);
                }
        }

        return NULL;
}


/* Determines whether instructon pipes to a file or runs in background*/
int handle_pipe_background(args_io_struct *instruction_io) {
        int pipe_loc = NOT_FOUND;
        int i;
        char *io_mode;

        for (i = 0; instruction_io -> instruction_list[i] != NULL; i++) {
                if (strcmp(instruction_io -> instruction_list[i], "&") == 0){
                        if (CONCURR){
                                fprintf (stderr, "Error: Shell in concurrent mode, background operation illegal\n");
                                return ERROR;
                        }
                        /* If there is an &, parses it from the instruction but sets flag
                        in struct for it to run in background */
                        if (instruction_io -> instruction_list[i+1] == NULL) {
                                instruction_io -> wait_status = BCKGRND;
                                instruction_io -> instruction_list[i] = NULL;
                        }
                        else {
                                fprintf(stderr, "Error: Misplaced & token\n");
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
        /* If there is no pipe location, simply return*/
        if(pipe_loc == NOT_FOUND){
                return SUCCESS;
        }
        else {
                /* If there is a pipe, attempt to open output file */
                int num_argu = (i - 1) - pipe_loc;
                if (num_argu == 1){
                        FILE *out_file = fopen(instruction_io -> instruction_list[pipe_loc+1], io_mode);
                        if (out_file == NULL) {
                                fprintf(stderr, "Error: Cannot open file.\n");
                                return ERROR;
                        }
                        else {
                                /* Store output FP in struct */
                                instruction_io -> instruction_list[pipe_loc] = NULL;
                                instruction_io -> output_file = out_file;
                                return SUCCESS;
                        }
                }
                else if (num_argu == 0) {
                        fprintf(stderr, "Error: No pipeline arguements.\n");
                        return ERROR;
                }
                else {
                        fprintf(stderr, "Error: Too many pipeline arguements.\n");
                        return ERROR;
                }
        }
}


/* Execute Individual Operations */
void execute_instructions(args_io_struct instruction_io) {
        char *instruction = instruction_io.instruction_list[0];
        pid_t child_pid = 0;

        /* CD command */
        if (strcmp(instruction, "cd") == 0) {
                /* If no directory is listed */
                if (instruction_io.instruction_list[1] == NULL) {
                        return;
                }
                /* If directory does not exist */
                else if (chdir(instruction_io.instruction_list[1]) == NOT_FOUND) {
                        fprintf(stderr, "%s: No such file or directory\n", instruction_io.instruction_list[1]);
                }
                else {
                        /* Resets PWD depending on new location */
                        char buffer[PATH_MAX];
                        setenv("PWD", getcwd(buffer, sizeof(buffer)), 1);
                }
        }
        /* If not CD, begins setup to launch process*/
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
/* Initializes child process and program invocation */
void launch(args_io_struct instruction_io, pid_t *child_pid) {
        char *prog_invoc = instruction_io.instruction_list[0];
        char **envp;
        make_env(&envp);

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
                        /* Switches output stream depending on pipe location */
                        dup2(fileno(instruction_io.output_file), 1);

                        int use_path = 1;
                        int i = 0;
                        /* If program invocation relies on loca directories */
                        while(prog_invoc[i] != '\0'){
                                if(prog_invoc[i] == '/'){
                                        use_path = 0;
                                }
                                i++;
                        }
                        /* Determines whether shell searches in local directory for
                        executable or in its PATH to the shell executable*/
                        if (use_path){
                                /* Uses path to shell executable directory to access 
                                executable */
                                char command[PATH_MAX];
                                strcpy(command, getenv("EXEC"));
                                strcat(command, prog_invoc);
                                if (execve(command, instruction_io.instruction_list, envp) == NOT_FOUND) {
                                        fprintf(stderr, "%s: command not found\n", prog_invoc);
                                        exit(EXIT_FAILURE);
                                }

                        } 
                        /* Searches locally for process */
                        else {
                                if (execve(prog_invoc, instruction_io.instruction_list, envp) == NOT_FOUND) {
                                        fprintf(stderr, "%s: command not found\n", prog_invoc);
                                        exit(EXIT_FAILURE);
                                }
                        }
                }
                
}

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
                fprintf(stderr, "%d terminated\n", (int)child_pid);
        }

        printf("Exitting\n");
        exit(EXIT_SUCCESS);
}