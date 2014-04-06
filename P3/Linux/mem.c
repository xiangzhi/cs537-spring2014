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

//struct for freeblock
typedef struct block_hd {
    int size;
    struct block_hd * next;
} block_t;

//struct for used block
typedef struct _used_t{
    int size;
    int magic;
} used_t;

//head pointer for freeblock
block_t* head;
//the size of the memory
int totalSize = 0;
//initialize error flag
int m_error = 0;
//minimum size of each block
int minimumSize = sizeof(block_t) + 8;

/**
 * Initalize memory of the size given
 */
int Mem_Init(int sizeOfRegion) {
    
    //check whether the size is larger than 0
    //also check whether mem_init has been initialized before
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

    //make sure it uses up a page
    pageSize = getpagesize();
    numPages = sizeOfRegion % pageSize;
    left = sizeOfRegion - (numPages * pageSize);
    left = pageSize - left;
    totalSize = sizeOfRegion;
    trueSize = sizeOfRegion + left;

    //initialize space
    fd = open("/dev/zero", O_RDWR);
    ptr = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED){ 
        perror("mmap"); exit(1); 
    }
    close(fd);

    //write the first block header
    head = (block_t *) ptr;
    head->size = trueSize;
    head->next = NULL;

    return 0;
}

/**
 * Allocate memory of size given by user
 * return NULL if not enough space
 */
void *Mem_Alloc (int size) {
    
    //make sure the size is more than 0
    if (size <= 0) {
        m_error = E_BAD_ARGS;
        return NULL;
    }

    //calculate the real size including header
    size = size + sizeof(used_t);
    //calculate the needed bytes to make the memory arrange by 8 bytes
    size = size + (8 - (size%8));

    block_t* ptr = head;
    block_t* best = NULL;
    block_t* prev = NULL;
    block_t* bestprev = NULL;

    //loop through the free list to find the best fit
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
                //first best block
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
    //calculate the remaining space
    int remaining = currSize - size;

    //check whether there is enough remaining space
    if(remaining > minimumSize){
        //create a new node for the remaining freeSpace
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
    //if there is not enough remaining space
    //add the space to the current block that returned to users.
    else{
        //if the block came from the head
        if(bestprev == NULL){
            head = next;
            header->size += remaining;
        }
        else{
            bestprev->next = next;
            header->size += remaining;
        }
    }

    //return the pointer to the freespace after header
    return ((char*) header) + sizeof(used_t);
}

/**
 * Free memory given by the pointer
 * return 0 on success, -1 on error
 * returns 0 if ptr is null
 */
int Mem_Free (void* ptr) {

    //check whether the pointer is NULL
	if(ptr == NULL){
		return 0;
	}
    
    //get the header of the free space
    used_t* header = (used_t*)(ptr - sizeof(used_t));

    //check whether the magic number is the same
    if(header->magic != MAGIC_NUM){
    	m_error = E_BAD_POINTER;
    	return -1;
    }

    //change the header to free block header
    int size = header->size;
    block_t* freeHeader = (block_t*) header;
    freeHeader->size = size;

    //insert the freespace into the list
 	block_t* itr = head;
    block_t* prev = NULL;

    //there is a possibility head is null, all space used up
    if(itr == NULL){
        freeHeader->next = head;
        head = freeHeader;
    }
    else{
        //loop through the whole list
        while(itr != NULL){
            if(freeHeader < itr){
                //if its above the head
                if(prev == NULL){
                    freeHeader->next = head;
                    head = freeHeader;
                    break;
                }
                //if its somewhere between two nodes
                else{
                    freeHeader->next = itr;
                    prev->next = freeHeader;
                    break;
                }
            }
            prev = itr;
            itr = itr->next;
        }
        //if its at the end of the list
        if(itr == NULL){
            prev->next = freeHeader;
            freeHeader->next = itr;
        }

    }
 	//coallase list
    //first check whether there is an node after it
    if(freeHeader->next != NULL){
        //check whether the next node is right below the current node.
        if((((char*)freeHeader) + freeHeader->size) == (char*)freeHeader->next){
            freeHeader->size += freeHeader->next->size;
            freeHeader->next = freeHeader->next->next;
        }
    }

    //check whether there is an node before it
    if(prev != NULL){
        //check whether the prev node is exactly above it.
        if((((char*)prev) + prev->size) == (char*)freeHeader){
                prev->size += freeHeader->size;
                prev->next = freeHeader->next;
            }        
    }
    //return success
    return 0;
}

/**
 * Debuging function
 * view the whole free list
 */
void Mem_Dump(){
    block_t* ptr = head;
    //total size allocated
    printf("total size:%d\n", totalSize);
    //loop through the first list
    while(ptr != NULL){
        printf("size: %d \t next:%p\n", ptr->size, ptr->next);
        ptr = ptr->next;
    }
    return;
}
