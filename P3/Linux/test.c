#include "mem.h"

int main(int argc, char *argv[]){
    Mem_Init(2048);
    Mem_Dump();
    char* ptr = (char*)Mem_Alloc(1024, 0);
    Mem_Dump();
    ptr = "LOL";
    return 0;
}