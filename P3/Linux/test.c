#include "mem.h"
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[]){
    Mem_Init(2048);
    char* ptr1 = (char*)Mem_Alloc(256);
    char* ptr2 = (char*)Mem_Alloc(512);
    char* ptr3 = (char*)Mem_Alloc(256);
    char* ptr4 = (char*)Mem_Alloc(512);
 	Mem_Dump();
    Mem_Free(ptr1);
    Mem_Free(ptr2);
	Mem_Dump();
	memcpy(ptr4,"LOL",strlen("LOL"));
    Mem_Free(ptr3);
    Mem_Free(ptr4);
	return 0;
}