#
# To compile, type "make" or make "all"
# To remove files, type "make clean"
#
OBJS = server.o request.o cs537.o client.o Pthread.o
TARGET = server

CC = gcc
CFLAGS = -g -Wall -Werror

LIBS = -lpthread 

.SUFFIXES: .c .o 

all: server client output.cgi

server: server.o request.o cs537.o Pthread.o
	$(CC) $(CFLAGS) -o server server.o request.o cs537.o Pthread.o $(LIBS)

client: client.o cs537.o Pthread.o
	$(CC) $(CFLAGS) -o client client.o cs537.o Pthread.o $(LIBS)

output.cgi: output.c
	$(CC) $(CFLAGS) -o output.cgi output.c

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm -f $(OBJS) server client output.cgi
