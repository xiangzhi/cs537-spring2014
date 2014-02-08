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
        if (strcmp(cmd, "quit()\n") == 0) {
            break;
        }
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

    return input;
}