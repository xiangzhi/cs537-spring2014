CC   = gcc
CFLAGS = -Wall -Werror -std=gnu99

#use to compile on Mac
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CFLAGS += -m32
endif

MFSLIBS = -lmfs

all: server libmfs.so client

# this generates the target executables
server: server.o udp.o fs.o
	$(CC) -o server server.o udp.o fs.o $(CFLAGS)

client: client.o
	$(CC) -o client client.o udp.o -L. $(CFLAGS) $(MFSLIBS)

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



