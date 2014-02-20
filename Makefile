# CREATED BY:  Xiang Zhi Tan, Section 2 

C     = gcc
CFLAGS = -O -Wall
PYTHON = -lpython2.6 -L /usr/lib/python2.6
PYTHONINCLUDE = -I /usr/include/python2.6

# argument CFLAGS is missing.  Place it here.


# fill in what is needed for target 'all'
all: lib.o mysh.o
	$(CC) $(CFLAGS) -o mysh mysh.o lib.o $(PYTHON)

lib: lib.c
	$(CC) -c -o lib.o lib.c $(CFLAGS)

mysh: mysh.c
	$(CC) -c -o mysh.o mysh.c $(CFLAGS) $(PYTHONINCLUDE)

clean:
	rm mysh *.o

