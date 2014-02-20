#include "lib.h"

char* substring(char* input, int start, int end){
    if(start >= end){
        return NULL;
    }

    if(strlen(input) < end){
        return NULL;
    }

    int size = end - start;
    char* newStr = (char*) malloc(sizeof(char) * (size + 1));
    int input_size = strlen(input);
    char* ptr = newStr;
    char* input_ptr = input;
    int i;
    for(i = 0; i < input_size; i ++){
        if(i >= start && i < end){
            *ptr = *input_ptr;
            ptr += sizeof(char);
        }
        input_ptr += sizeof(char);
    }
    ptr = '\0';
    return newStr;
}

int indexOf(char* input, char _char){
    char* ptr = input;
    int i;
    for(i = 0; i < strlen(input); i++){
        if(*ptr == _char){
            return i;
        }
        ptr += sizeof(char);
    }
    return -1;
}

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