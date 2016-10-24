# Makefile for Project 1 COMP 111
#   by 
#    	Justin Tze Tsun Lee
#   	Matthew Carrington-Fair
#     	Tomer Shapira 

############## Variables ###############

CC = gcc # The compiler being used

# Compile flags

CFLAGS = -g -std=gnu99 -Wall -Wextra -Werror -Wfatal-errors -pedantic -lpthread -lreadline


############### Rules ###############

all: shell echo pause clr dir help environ 

## Compile step (.c files -> .o files)

# To get *any* .o file, compile its .c file with the following rule.
%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

## Linking step (.o -> executable program)
shell: shell.o shell_util.o shell_ops.o
	$(CC) -o shell shell.o shell_util.o shell_ops.o $(CFLAGS) 
echo: echo.o
	$(CC) -o echo echo.o $(CFLAGS) 
pause: pause.o
	$(CC) -o pause pause.o $(CFLAGS) 
clr: clr.o
	$(CC) -o clr clr.o $(CFLAGS) 
dir: dir.o
	$(CC) -o dir dir.o $(CFLAGS)
help: help.o
	$(CC) -o help help.o $(CFLAGS)
environ: environ.o
	$(CC) -o environ environ.o $(CFLAGS)
clean:
	rm -f *.o core* *~shell *~echo *~pause *~clr *~help *~environ