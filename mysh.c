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
int execute(char**, int);
int parseArgv(char*, char***);


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

        //parse the input
        char** exec_args;
        int num_argv = parseArgv(cmd, &exec_args);

        //check whether we reach the quit phase
        if (strcmp(exec_args[0], "exit") == 0) {
            free(exec_args);
            exit(0);
        }

        execute(exec_args, num_argv);
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
    //the last argument is a NULL
    list[index] = NULL;
    index++;

    free(pointer);
    *exec_args = list;
    return index;
}
