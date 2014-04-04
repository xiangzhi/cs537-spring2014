# CREATED BY:  Xiang Zhi Tan, Section 2 

C     = gcc
CFLAGS = -Werror -Wall
PYTHON = -lpython2.6 -L /usr/lib/python2.6
PYTHONINCLUDE = -I /usr/include/python2.6

# fill in what is needed for target 'all'
all: mem.o
	$(CC) -shared -o libmem.so mem.o

libmem: mem.c
	$(CC) -c - fpic mem.c $(CFLAGS)


clean:
	rm -f *.o *.so

