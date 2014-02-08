#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
//max size is 514 because 512 character + '\n' + '\0'
#define MAX_SIZE 514

//function prototypes
void displayError(void);
char* prompt(void);
int execute(char*);
char** parseArgv(char*);

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
        //check whether we reach the quit phase
        if (strcmp(cmd, "quit()") == 0) {
            break;
        }
        execute(cmd);
    }
    printf("quitting\n");
    return 0;
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
int execute(char* input){
    //here we should parse the input and do something about it.

    //call fork
    int rc  = fork();
    //check whether fork was succesful
    if(rc < 0){
        displayError();
        exit(1);
    }

    //this is the child
    if(rc == 0){

        char** exec_args = parseArgv(input);

        //execvp should have never retunr
        execvp(exec_args[0], exec_args);
        //the command should never reach here
        displayError();
        return 1;
    }
    //this is the parent
    else{
        //wait untill anyone of the children finishs
        int wc = wait(NULL);
        return 0;
    }
}

char** parseArgv(char* input){
    //count the number of arguments in the list;
    int num_argv = 0;
    char* ptr = input;
    do{
        num_argv++;
        ptr = strchr(ptr, ' ');
    }
    while(ptr != NULL);
    printf("arguments:%d", num_argv);

    char** list = (char**) malloc(sizeof(char*) * num_argv);
    list[0] = "ls";
    list[1] = NULL;
    return list;
}
