/* CS537 - Spring 2014 - Program 5
 * CREATED BY:  
 * Xiang Zhi Tan (xtan@cs.wisc.edu)
 * Roy Fang (fang@cs.wisc.edu)
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#ifndef __fs__
#define __fs__

#define FS_DIRECTORY    (0)
#define FS_REGULAR_FILE (1)
#define BUFFER_SIZE (4096)
#define NAME_LENGTH (60)

//to store checkpoint region
typedef struct _checkPoint{
    int endPtr;
    int mapPtrs[256];
}checkPoint;

//to store iMaps
typedef struct _iMap{
    int ptr[16];
}iMap;

//to store  iNodes
typedef struct _iNode{
    int size;
    int type;
    int dataPtrs[14];
}iNode;

//the stat information, similar to mfs's stat
typedef struct _stat {
    int type;   // MFS_DIRECTORY or MFS_REGULAR
    int size;   // bytes
} stat_t;

//to store individual directories
typedef struct _dir{
    char name[60];
    int pair;
}fs_dir;

//to store the whole list in each data region
typedef struct _dir_list{
    fs_dir list[64];
}fs_dir_list;

/**
 * fs_init
 * initialize the disk with the fileName given
 * - fileName: name of the file to store the disk
 * return: 0 for success, -1 if failed
 */
int fs_init(char* filename);

/**
 * fs_close
 * close the file descripter and clean up
 * return: 0 for success, -1 if failed
 */
int fs_close();

/**
 * fs_create
 * create a new file or directory
 * variables are not checked, programmers job to make sure its valid
 * - pinum: parent's iNode number
 * - type: type of file to be created
 * - name: name of the file
 * return: 0 for success, -1 if failed
 */
int fs_create(int pinum,  int type, char *name);

/**
 * fs_lookup
 * looksup the file with the name in the directory
 * - pinum: parent's iNode number
 * - name: name of the file to be searched for
 * return: if success, iNode number of file,  -1 if failed
 */
int fs_lookup(int pinum, char *name);

/**
 * fs_stat
 * the stat file of the given iNode number
 * - inum: the iNode number
 * - m: pointer to store the stat_t file
 * return: 0 for success, -1 if failed
 */
int fs_stat(int inum, stat_t *m);

/**
 * fs_write
 * write a 4096 byte at the defined block
 * - inum: file to be written
 * - buffer: buffer to be written to block
 * - block: which block to write to
 * return: 0 for success, -1 if failed
 */
int fs_write(int inum, char *buffer, int block);

/**
 * fs_read
 * read a 4096 byte at the defined block
 * - inum: file to be read
 * - buffer: buffer to store result
 * - block: which block to read
 * return: 0 for success, -1 if failed
 */
int fs_read(int inum, char *buffer, int block);

/**
 * fs_unlick
 * unlink the file 
 * - pinum: parent's iNode number
 * - name: name of the file to be deleted
 * return: if success, iNode number of file,  -1 if failed
 */
int fs_unlink(int pinum, char *name);

/**
 * fs_sync
 * push all change to the disk
 * return: if success, iNode number of file,  -1 if failed
 */
int fs_fsync();

#endif