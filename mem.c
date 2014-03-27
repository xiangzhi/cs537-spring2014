#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "mem.h"

typedef struct block_hd {
    int size;
    struct block_hd * next;
} block_t;

block_t* head;

int Mem_Init (int sizeOfRegion) {
    
    if (sizeOfRegion <= 0) {
        m_error = E_BAD_ARGS;
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
    
    assert(ptr != NULL);
    
    head = (block_t *) ptr;
    head->size = trueSize - sizeof(block_t);
    head->next = NULL;
    
    return 0;
}
    
void *Mem_Alloc (int size, int style) {
    
    if (style != 0) {
        m_error = E_BAD_ARGS;
        return NULL;
    }
    
    if (size <= 0) {
        m_error = E_BAD_ARGS;
        return NULL;
    }
    return NULL;
    
    block_t* temp = head;
    block_t* best;
    block_t* prev;
}

int Mem_free (void* ptr) {
    
    return 0;
}
    
void Mem_Dump() {
    
    return;
}
