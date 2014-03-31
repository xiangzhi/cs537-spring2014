#include "mem.h"

int main(int argc, char *argv[]){
    Mem_Init(2048);
    Mem_Dump();
    void* ptr = Mem_Alloc(1024, 0);
    Mem_Dump();
    return 0;
}