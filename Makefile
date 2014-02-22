# CREATED BY:  Xiang Zhi Tan, Section 2 

C     = gcc
CFLAGS = -O -Wall
PYTHON = -lpython2.6 -L /usr/lib/python2.6
PYTHONINCLUDE = -I /usr/include/python2.6

# argument CFLAGS is missing.  Place it here.


# fill in what is needed for target 'all'
all: string_lib.o array_lib.o mysh.o
	$(CC) $(CFLAGS) -o mysh mysh.o array_lib.o string_lib.o $(PYTHON)

string_lib: string_lib.c
	$(CC) -c -o string_lib.o string_lib.c $(CFLAGS)

array_lib: array_lib.c
	$(CC) -c -o array_lib.o array_lib.c $(CFLAGS)

mysh: mysh.c
	$(CC) -c -o mysh.o mysh.c $(CFLAGS) $(PYTHONINCLUDE)

clean:
	rm -f mysh *.o

