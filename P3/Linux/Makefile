# CS537 - Spring 2014 - Program 2
# CREATED BY:  
# Xiang Zhi Tan (xtan@cs.wisc.edu)
# Roy Fang (fang@cs.wisc.edu)
#

CC     = gcc
CFLAGS = -Werror -Wall

libmem.so: mem.o
	$(CC) -shared -o libmem.so mem.o

mem.o: mem.c
	$(CC) -c -fpic mem.c $(CFLAGS)

clean:
	rm -f *.o *.so

