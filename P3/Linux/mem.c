#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include "mem.h"
#define MAGIC_NUM 50

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
int totalSize = 0;

//initialize error flag
int m_error = 0;

int Mem_Init(int sizeOfRegion) {
    
    if (sizeOfRegion <= 0 || totalSize != 0) {
        m_error = E_BAD_ARGS;
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
    totalSize = sizeOfRegion;
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
    
void *Mem_Alloc (int size) {
    
    if (size <= 0) {
        m_error = E_BAD_ARGS;
        return NULL;
    }
    size = size + sizeof(used_t);
    //calculate the needed bytes to make the memory arrange by 8 bytes
    size = size + (8 - (size%8));

    block_t* ptr = head;
    block_t* best = NULL;
    block_t* prev = NULL;
    block_t* bestprev = NULL;

    while(ptr != NULL){
        if(ptr->size >= size){
            //if there is a best before
            if(best != NULL){
                //make sure the ptr is smaller than current best
                if(best->size > ptr->size){
                    bestprev = prev;
                    best = ptr;
                }
            }
            else{
                bestprev = prev;
                best = ptr;
            }
        }
        prev = ptr;
        ptr = ptr->next;
    }
    //no best space
    if(best == NULL){
        m_error = E_NO_SPACE;
        return NULL;
    }
    //save the value of the variable of the selected block
    int currSize = best->size;
    block_t* next = best->next;
    //allocate space for the used header
    used_t* header = (used_t*) best;
    header->size = size;
    header->magic = MAGIC_NUM;

    //find the first byte that is free
    if(currSize - size > 0){
        ptr = (block_t*)(((void*) header) + size);
        ptr->size = currSize - size;
        ptr->next = next;
        //making sure whether there is a previous value
        if(bestprev == NULL){
            //if there is no prev, set the head to the new ptr;
            head = ptr;
        }
        else{
            //set the previous node's next to this node
            bestprev->next = ptr;
        }
    }
    else{
        if(bestprev == NULL){
            //if there is no prev, set the head to the new ptr;
            head = next;
        }
        else{
            //set the previous node's next to this node
            bestprev->next = next;
        }
    }

    return ((char*) header) + sizeof(used_t);
}

int Mem_Free (void* ptr) {

	if(ptr == NULL){
		return 0;
	}
    
    used_t* header = (used_t*)(ptr - sizeof(used_t));
    if(header->magic != MAGIC_NUM){
    	m_error = E_BAD_POINTER;
    	return -1;
    }

    //make a new header
    int size = header->size;
    block_t* freeHeader = (block_t*) header;
    freeHeader->size = size;

    //go through the freelist and insert the value
 	block_t* itr = head;
 	block_t* prev = NULL;
 	while(itr != NULL){
 		if(freeHeader < itr){
 			if(prev == NULL){
 				freeHeader->next = head;
 				head = freeHeader;
 				break;
 			}
 			else{
 				freeHeader->next = itr;
 				prev->next = freeHeader;
 				break;
 			}
 		}
 		prev = itr;
 		itr = itr->next;
 	}

 	//coallase list
 	block_t* next = freeHeader->next;

 	if((((char*)freeHeader) + freeHeader->size) == (char*)next){
 		freeHeader->size += next->size;
 		freeHeader->next = next->next;
 	}
 	//this mean the prev and freelist are connected
 	if(prev == NULL){
 		prev = head;
 	}
 	if((((char*)prev) + prev->size) == (char*)freeHeader){
 		prev->size += freeHeader->size;
 		prev->next = freeHeader->next;
 	}

    return 0;
}
    
void Mem_Dump(){
    block_t* ptr = head;
    printf("total size:%d\n", totalSize);
    while(ptr != NULL){
        printf("size: %d \t next:%p\n", ptr->size, ptr->next);
        ptr = ptr->next;
    }
    return;
}
