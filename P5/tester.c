#include "fs.h"
#include <stdio.h>

int fs_init(char* filename);
int fs_lookup(int pinum, char *name);
int fs_close();
int fs_stat(int inum, stat_t *m);
void fs_print();

int main(int argc, char *argv[]){
    fs_init(argv[1]);
    /*
    printf("find:, result%d\n", fs_lookup(0, "."));
    printf("find:, result%d\n", fs_lookup(0, ".."));
    printf("find:, result%d\n", fs_lookup(0, "dir"));
    printf("find:, result%d\n", fs_lookup(0, "code"));
    stat_t stat;
    fs_stat(fs_lookup(0, "code"),&stat);
    printf("type:%d, size:%d\n", stat.type, stat.size);
    fs_stat(3,&stat);
    printf("type:%d, size:%d\n", stat.type, stat.size);
    */
    fs_print();
    fs_close();
    return 0;
}