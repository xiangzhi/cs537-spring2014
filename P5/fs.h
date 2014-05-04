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
    int type;
    int size;
    int dataPtrs[14];
}iNode;

typedef struct _dir{
    char name[60];
    int pair;
}dir;

void fs_init(char* filename);

int fs_close();

int fs_create(int pinum,  int type, char *name);

int fs_lookup(int pinum, char *name);

void fs_print();

#endif