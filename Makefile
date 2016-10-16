# Makefile for Project 1 COMP 111
# Matthew Carrington-fair

############## Variables ###############

CC = gcc # The compiler being used

# Compile flags

CFLAGS = -g -std=gnu99 -Wall -Wextra -Werror -Wfatal-errors -pedantic -lpthread -lreadline

############### Rules ###############

all: shell echo pause

## Compile step (.c files -> .o files)

# To get *any* .o file, compile its .c file with the following rule.
%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

## Linking step (.o -> executable program)

shell: shell.o
	$(CC) -o shell shell.c $(CFLAGS) 
echo: echo.o
	$(CC) -o echo echo.c $(CFLAGS) 
pause: pause.o
	$(CC) -o pause pause.c $(CFLAGS) 

clean:
	rm -f *.o core* *~shell *~echo *~pause