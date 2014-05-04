#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#ifndef __fs__
#define __fs__

#define FS_DIRECTORY    (0)
#define FS_REGULAR_FILE (1)


typedef struct _checkPoint{
    int endPtr;
    int mapPtrs[256];
}checkPoint;

typedef struct _iMap{
    int ptr[16];
}iMap;

typedef struct _iNode{
    int size;
    int type;
    int dataPtrs[14];
}iNode;

typedef struct _stat {
    int type;   // MFS_DIRECTORY or MFS_REGULAR
    int size;   // bytes
} stat_t;

typedef struct _dir{
    char name[60];
    int pair;
}fs_dir;

typedef struct _dir_list{
    fs_dir list[64];
}fs_dir_list;

void fs_init(char* filename);

int fs_close();

int fs_create(int pinum,  int type, char *name);

int fs_lookup(int pinum, char *name);

int fs_stat(int inum, stat_t *m);

int fs_write(int inum, char *buffer, int block);
int fs_read(int inum, char *buffer, int block);
int fs_unlink(int pinum, char *name);

void fs_print();

#endif