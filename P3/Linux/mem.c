#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include "mem.h"
#define MAGIC_NUM 50;

typedef struct block_hd {
    int size;
    struct block_hd * next;
} block_t;

typedef struct _used_t{
    int size;
    int magic;
} used_t;

block_t* head;
int MagicNum = MAGIC_NUM;

int Mem_Init(int sizeOfRegion) {
    
    if (sizeOfRegion <= 0) {
        //m_error = E_BAD_ARGS;
        printf("Size must be greater than 0");
        return -1;
    }
    
    int pageSize;
    int left;
    int numPages;
    int trueSize;
    
    void* ptr;
    int fd;

    pageSize = getpagesize();
    numPages = sizeOfRegion % pageSize;
    left = sizeOfRegion - (numPages * pageSize);
    left = pageSize - left;
    
    trueSize = sizeOfRegion + left;

    
    fd = open("/dev/zero", O_RDWR);
    ptr = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED){ 
        perror("mmap"); exit(1); 
    }
    close(fd);

    head = (block_t *) ptr;
    //head->size = trueSize - sizeof(block_t);
    //write(1, "test\n", sizeof("test\n"));
    head->size = trueSize;
    head->next = NULL;

    return 0;
}
    
void *Mem_Alloc (int size, int style) {
    
    if (style != 0) {
        //m_error = E_BAD_ARGS;
        return NULL;
    }
    
    if (size <= 0) {
        //m_error = E_BAD_ARGS;
        return NULL;
    }
    return NULL;
    
    int neededSize = size + sizeof(used_t);
    //calculate the needed bytes to make the memory arrange by 8 bytes
    neededSize = neededSize + (neededSize%8);

    block_t* ptr = head;
    block_t* best = head;
    block_t* prev = NULL;

    while(ptr != NULL){
        if(ptr->size >= neededSize && ptr->size < best->size){
            best = ptr;
        }
        prev = ptr;
        ptr = ptr->next;
    }

    //save the value of the variable of the selected block
    int currSize = best->size;
    block_t* next = best->next;

    used_t* header = (used_t*) best;
    header->size = size + sizeof(used_t);
    header->magic = MAGIC_NUM;
    //find the first byte that is free
    ptr = (block_t*)(((char*) header) + size + sizeof(used_t));

    ptr->size = currSize - size - sizeof(used_t);
    ptr->next = next;
    //making sure whether there is a previous value
    if(prev == NULL){
        //if there is no prev, set the head to the new ptr;
        head = ptr;
    }
    else{
        //set the previous node's next to this node
        prev->next = ptr;
    }

    return ((char*) ptr) + sizeof(used_t);
}

int Mem_free (void* ptr) {
    
    return 0;
}
    
void Mem_Dump() {
    
    return;
}