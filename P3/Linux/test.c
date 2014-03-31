#include "mem.h"

int main(int argc, char *argv[]){
    Mem_Init(2048);
    void* ptr = Mem_Alloc(1024, 0);
    return 0;
}