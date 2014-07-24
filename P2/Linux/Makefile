# CS537 - Spring 2014 - P2
# CREATED BY:  
# Xiang Zhi Tan (xtan@cs.wisc.edu)
# Roy Fang (fang@cs.wisc.edu)

C     = gcc
CFLAGS = -O -Wall
PYTHON = -lpython2.6 -L /usr/lib/python2.6
PYTHONINCLUDE = -I /usr/include/python2.6

# fill in what is needed for target 'all'
all: mysh.o
	$(CC) $(CFLAGS) -o mysh mysh.o $(PYTHON)

mysh: mysh.c
	$(CC) -c -o mysh.o mysh.c $(CFLAGS) $(PYTHONINCLUDE)

clean:
	rm -f mysh *.o

