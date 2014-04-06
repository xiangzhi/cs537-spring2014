#include "mem.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[]){
    Mem_Init(2048);
    void* ptr[9];
    ptr[0] = Mem_Alloc(100);
    ptr[1] = Mem_Alloc(20);
    ptr[2] = Mem_Alloc(10);
    ptr[3] = Mem_Alloc(100);
    Mem_Dump();
    Mem_Free(ptr[0]);
    Mem_Free(ptr[2]);
    ptr[2] = Mem_Alloc(3);
    Mem_Dump();
	return 0;
}