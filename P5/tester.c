#include "fs.h"
#include <stdio.h>

void fs_init(char* filename);

int main(int argc, char *argv[]){
    printf("hello");
    fs_init(argv[1]);
    return 0;
}