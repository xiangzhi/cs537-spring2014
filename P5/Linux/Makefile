# CS537 - Spring 2014 - Program 5
# CREATED BY:  
# Xiang Zhi Tan (xtan@cs.wisc.edu)
# Roy Fang (fang@cs.wisc.edu)

CC   = gcc
# used GCC99 because of using <stdbool.h>
CFLAGS = -Wall -Werror -std=gnu99

#use to compile on Mac
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CFLAGS += -m32
endif

MFSLIBS = -lmfs

all: server libmfs.so 

# this generates the target executables
server: server.o udp.o fs.o
	$(CC) -o server server.o udp.o fs.o $(CFLAGS)

fs.o: fs.c fs.h
	$(CC) -c -fpic fs.c fs.h $(CFLAGS)

libmfs.so: mfs.o udp.o
	$(CC) -shared -o libmfs.so mfs.o udp.o $(CFLAGS)

mfs.o: mfs.c mfs.h
	$(CC) -c -fpic mfs.c mfs.h $(CFLAGS)

udp.o: udp.h udp.c
	$(CC) -c -fpic udp.c udp.h $(CFLAGS)

clean:
	rm -f server client *.so *.o *.gch



