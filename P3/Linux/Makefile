# CREATED BY:  Xiang Zhi Tan, Section 2 

CC     = gcc
CFLAGS = -Werror -Wall

# fill in what is needed for target 'all'
libmem.so: mem.o
	$(CC) -shared -o libmem.so mem.o

mem.o: mem.c
	$(CC) -c -fpic mem.c $(CFLAGS)

clean:
	rm -f *.o *.so

