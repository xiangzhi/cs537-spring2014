CC   = gcc
CFLAGS = -Wall -Werror -m32 -std=gnu99

FSLIBS = -lfs
MFSLIBS = -lmfs

all: server client libfs.so tester.o libmfs.so

#tester removed this
tester.o: tester.c libfs.so
	$(CC) -o tester.o tester.c $(CFLAGS) -L. $(FSLIBS)

# this generates the target executables
server: server.o udp.o libfs.so
	$(CC) -o server server.o udp.o $(CFLAGS) -L. $(FSLIBS)

client: client.o libmfs.so
	$(CC) -o client client.o $(CFLAGS)  -L. $(MFSLIBS)

libfs.so: fs.o
	$(CC) -shared -o libfs.so fs.o $(CFLAGS)

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



