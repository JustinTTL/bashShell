# bashShell
Interactive and Batch Shell Implementation

Design Overview:

	List of Files:
			shell.c : File containing main
			# Utility files #
			shell_util.c
			shell_util.h
			# Shell operations files #
			shell_ops.h
			shell_ops.c
			# Supported commands #
			clr.c
			dir.c
			echo.c
			environ.c
			help.c
			manual.txt
			pause.c
			# Makefile #
			Makefile

	Our code implements a shell that can execute the following: cd, clr, dir, echo, environ, help, pause, and quit. 
	All of the calls except for cd and quit have their own .c file. They are called as child processes within 
	shell.c. 

	shell.c is the main file; it handles executing the inputted commands (from the command line or a batchfile). 
	shell_util sets up all of the evironment variables and parses through the string that is read in as the 
	command. Core shell operations are performed within shell_ops. This includes reading in input, processing input,
	executing instructions and launching child processes. 

	The input is split according to strings/characters that indicate a certain action ("/", ";", "./", 
	"&", ">", ">>", and "\n"). It also strips any " " or "\t" characters out of commands.

	In shell_func.h we contain the structure "args_io_struct" which is setup as so:

			typedef struct args_io_struct{
				char **instruction_list;
				FILE *output_file;
				int wait_status;
			} args_io_struct;

	This is used to represent a single command as a list of instructions (separated by semi colons),
	an output_file (by default stdout, but changes through piping), and a wait_status (whether process runs
	in background or foreground, by default parent does wait). If there is a change in I/O, it will be stored
	in this file pointer which will be used to change stdout to correct location when launching a child process.
	Similarly, if a parent is not waiting for a child, we have a background signal handler to prevent any zombie
	processes, and that will alert when a child	is finished, and allow the parent to indicate that the process
	has terminated.

	The general flow of our code goes as follows:

	main -> run_shell - > process_instructions -> handle_instructions -> handle_pipe_background -> 
	execute_instructions -> launch -> run_shell

	There are specific cases or commands that change the cycle (quit, cd, etc), and there are several helper functions
	or sub operations that perform other initialization or setup.

	When exiting our shell, we ensure that all memory is returned and we call our quit_shell method which waits for any
	child processes to terminate before exiting.


Specification:

	If the command to be read in is blank, the program does nothing. Our shell handles lines that have no commands
	between semi-colons as well. Our shell will also handle extra spaces or tabs between instructions. 

	The biggest design choices came when figuring how to configure our environment variables for our shell and handling
	program invocation. The varying version of the exec family of functions allowed different implementation options.
	In the end we decided upon using execve, and passing in all necessary environment variables including parent, shell,
	our "PATH" aptly named EXEC and our default PATH passed into the original shell. Then, we made sure to check the
	given program to invoke and determine whether we were to search locally (within current or directory given by command)
	or within our EXEC path back to our shell directory. That way we could still have functionality in some of our commands
	by using the default PATH variable but not "gain too much power" by being able to access more of its bin utilities. We
	would derive our EXEC path upon shell initialization deriving it from the shell environment variable we set. 
 
	All supported commands operate as indicated by the specifications. 

