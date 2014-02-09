#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern char* substring(char* input, int start, int end);
extern int indexOf(char* input, char search_char);
int arrayRemove(char*** _array, int size, int position);