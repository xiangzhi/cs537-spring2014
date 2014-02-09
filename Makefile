# CREATED BY:  Xiang Zhi Tan, Section 2 

C     = gcc
CFLAGS = -O -Wall
PYTHON = -lpython2.6

# argument CFLAGS is missing.  Place it here.


# fill in what is needed for target 'all'
all: lib.o mysh.o
	$(CC) $(CFLAGS) -o mysh mysh.o lib.o $(PYTHON)

cachesim: lib.c
			$(CC) -c -o lib.o lib.c $(CFLAGS)

cache: mysh.c
			$(CC) -c -o mysh.o mysh.c $(CFLAGS)

