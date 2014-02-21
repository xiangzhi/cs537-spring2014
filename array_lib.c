#include "array_lib.h"

int arrayRemove(char*** _array, int size, int position){

    if(position >= size){
        return -1;
    }

    char** array = *_array;
    char** newArray = (char**) malloc (sizeof(char**) * (size-1));
    char** ptr = newArray;
    int i;
    for(i = 0; i < size; i++){
        if(i == position){
            continue;
        }
        *ptr = array[i];
        ptr += sizeof(char);
    }
    free(array);
    *_array = newArray;
    return 0;
}