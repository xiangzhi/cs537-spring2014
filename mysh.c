#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <Python/Python.h>
#include "lib.h"
//max size is 514 because 512 character + '\n' + '\0'
#define MAX_SIZE 514

//function prototypes
void displayError(void);
char* prompt(void);
int execute(char**, int);
int parseArgv(char*, char***);
int buildIn(char**, int);
void execPython(char*, int, char**);
void redirectOutput(void);
void setOutput(void);
void closeOutput(void);

int redirectFlag;
char* redirectFileName;
int output_fd;

int main(int argc, char* argv[]) {
    
    // Starting mysh program with incorrect number of arguments
    if (argc > 2) {
        printf("%d\n", argc);
        displayError();
        exit(1);
    }
    
    while (1) {
        char* cmd = prompt();

        //check whether prompt was successful
        if(cmd == NULL){
            continue;
        }

        //restart flgs
        redirectFlag = -1;

        //parse the input
        char** exec_args;
        int num_argv = parseArgv(cmd, &exec_args);
        if(num_argv == -1){
            continue;
        }
        //change the STDIO if needed
        setOutput();
        int check = buildIn(exec_args, num_argv);
        if(check == 0 || check == 2){
            closeOutput();
            free(exec_args);
            continue;
        }

        check = execute(exec_args, num_argv);
        free(exec_args);
    }
    //this is an infinite loop
    displayError();
    return 1;
}

//display same error
void displayError(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

// Prompts and retreives and returns user input
char* prompt() {
    char* input = (char*) malloc(MAX_SIZE);
    printf("mysh>");
    //get the input from STDIN
    input = fgets(input, MAX_SIZE, stdin);
    //check whether fgets is successful
    if(input == NULL){
        displayError();
        return NULL;
    }
    //find first instance of '\n'
    char* new_line = strchr(input, '\n');
    //means newline is not in the string
    //which implies the string is longer than 512
    if(new_line == NULL){
        displayError();
        return NULL;
    }
    //replace new_line with terminate string
    *new_line = '\0';
    //remove the empty string infront of the first character
    while(*input == ' '){
        input += sizeof(char);
        //this means all the input was white space
        //there is no command,  so we return null
        //and skip to the next prompt
        if(*input == '\0'){
            return NULL;
        }
    }
    return input;
}

/*
 * function that deal with execution of function
 */
int execute(char** exec_args, int num_argv){

    //close the open file
    closeOutput();
    //call fork
    int rc  = fork();
    //check whether fork was succesful
    if(rc < 0){
        displayError();
        exit(1);
    }

    //this is the child
    if(rc == 0){
        if(redirectFlag == 1){
            redirectOutput();
        }
        //execvp should have never retunr
        execvp(exec_args[0], exec_args);
        //the command should never reach here
        displayError();
        exit(1);
    }
    //this is the parent
    else{
        //wait untill anyone of the children finishs
        int wc = wait(NULL);
        return 0;
    }
}

int parseArgv(char* input, char*** exec_args){
    //count the number of arguments in the list;
    char* pointer = (char*) malloc( strlen(input) * sizeof(char) );
    pointer = strncpy(pointer, input, strlen(input) * sizeof(char) );
    //probably memory problem
    if(pointer == NULL){
        displayError();
        exit(1);
    }

    //get how many arguments are in the input
    int num_argvs = 0;
    char* token = NULL;
    token = strtok(pointer, " ");
    while(token != NULL)
    {
        num_argvs++;
        token = strtok(NULL, " ");
    }

    //allocate an array to store the arguments
    char** list = (char**) malloc(sizeof(char*) * (num_argvs + 1));

    //copy the tokens into the string
    int index = 0;
    token = NULL;
    token = strtok(input, " ");
    while(token != NULL)
    {
        list[index] = token;
        index++;
        token = strtok(NULL, " ");
    }
    
    //check for the redirect flag
    for(int i = 0; i < index; i++){
        char* word = list[i];
        int char_index = indexOf(word, '>');
        if(char_index != -1){
            if(redirectFlag == 1){
                displayError();
                return -1;
            }

            //case zero the > is by itself
            if(strlen(word) == 1){
                redirectFileName = list[i+1];
                //remove twice
                arrayRemove(&list, index, i + 1);
                arrayRemove(&list, index, i);
                index -= 2;
                i -= 2;
            }
            //first case, the < starts the fileName -> ls >text.txt
            else if(char_index == 0 ){
                redirectFileName = substring(word,1, strlen(word));

                //remove the fileName from the list
                arrayRemove(&list, index, i);
                index--;
                i--;
            }
            //second case,  where < is at the end -> ls> text.txt
            else if(char_index == (strlen(word) - 1) ){

                //if the > exsist in the end without a file Name
                if((i + 1) == index){
                    displayError();
                    return -1;
                }

                list[i] = substring(word, 0, strlen(word) -1);
                redirectFileName = list[i + 1];

                //remove the fileName from the list
                arrayRemove(&list, index, i+1);
                index--;
                i--;
            }
            //last case,  where  > is in the middle
            else{
                list[i] = substring(word, 0, char_index);
                redirectFileName = substring(word, char_index + 1, strlen(word));
            }
            redirectFlag = 1;
            printf("filename:%s\n", redirectFileName);
        }
    }

    //the last argument is a NULL
    list[index] = NULL;

    //debuging to see what command is left in the system;
    for(int i = 0; i < index; i++){
        printf("%s\n",list[i]);
    }

    free(pointer);
    *exec_args = list;
    return index;
}

int buildIn(char** exec_args, int num_argv){
    char* command = exec_args[0];

    //check whether we reach the quit phase
    if (strcmp(command, "exit") == 0) {
        free(exec_args);
        exit(0);
    }
    
    // Check for .py substring
    if (strstr(command, ".py") != NULL) {
        printf("Running Python");
        execPython(command, num_argv, exec_args);
        return 0;
    }

    
    //check for pwd
    if (strcmp(command, "pwd") == 0) {
        char* path = getcwd(NULL, 0);
        write(output_fd, path, strlen(path));
        write(output_fd, "\n", 1);
        free(path);
        return 0;
    }
    //check for cd
    if (strcmp(command, "cd") == 0){
        if(num_argv > 2){
            displayError();
            return 2;
        }
        char* path;
        if(num_argv == 1){
            path = getenv("HOME");
        }
        else{
            path = exec_args[1];
        }
        if(path == NULL){
            displayError();
            exit(1);
        }
        //try change directories
        if(chdir(path) != 0){
            displayError();
            return 2;
        }
        return 0;
    }

    return 1;
}

void setOutput(void){
    if(redirectFlag == 1){
        output_fd = open(redirectFileName, 
            O_RDWR | O_TRUNC | O_CREAT, S_IRWXU);
    }
    else{
        output_fd = STDIN_FILENO;
    }
}

void closeOutput(void){
    if(redirectFlag == 1){
        close(output_fd);
        output_fd = STDIN_FILENO;
    }
}

void redirectOutput() {
    int close_rc = close(STDOUT_FILENO);
    if (close_rc < 0) {
        displayError();
        exit(1);
    }

    int fd = open(redirectFileName, O_RDWR | O_TRUNC | O_CREAT, S_IRWXU);
    printf("fd:%d\n",fd);
    if (fd < 0) {
        displayError();
        exit(1);
    }
}

void execPython(char* fileName, int argc, char* argv[]) {
    FILE* fileptr = fopen(fileName, "rb");
    if (fileptr == NULL) {
        printf("script could not be found %s", fileName);
        exit(1);
    }
    
    Py_SetProgramName(fileName);
    Py_Initialize();
    PySys_SetArgv(argc, argv);
    PyRun_SimpleFile(fileptr, fileName);
    Py_Finalize();
    fclose(fileptr);
    
    printf("success\n");
    return;
}
