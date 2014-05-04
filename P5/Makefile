CC   = gcc
CFLAGS = -Wall -Werror -m32 -std=gnu99

FSLIBS = -lfs

all: server client libfs.so tester.o

#tester removed this
tester.o: tester.c libfs.so
	$(CC) -o tester.o tester.c $(CFLAGS) -L. $(FSLIBS)

# this generates the target executables
server: server.o udp.o
	$(CC) -o server server.o udp.o $(CFLAGS)

client: client.o udp.o
	$(CC) -o client client.o udp.o $(CFLAGS)

libfs.so: fs.o
	$(CC) -shared -o libfs.so fs.o $(CFLAGS)

fs.o: fs.c fs.h
	$(CC) -c -fpic fs.c fs.h $(CFLAGS)

clean:
	rm -f server client *.so *.o



