gcc -c -fpic mem.c -Wall -Werror
gcc -shared -o libmem.so mem.o
gcc -lmem -L. -o prog test.c -Wall -Werror
prog
