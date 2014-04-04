#include "mem.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[]){
    assert(Mem_Init(1) == 0);
    assert(Mem_Alloc(4048) != NULL);
	return 0;
}