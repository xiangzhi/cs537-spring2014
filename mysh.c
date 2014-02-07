#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#define BUFFER 512

char error_message[30] = "An error has occurred\n";
char input[BUFFER];

// Prompts and retreives and returns user input
char* prompt() {
    printf("mysh>");
    scanf("%s", input);
    return input;
}

int main(int argc, char* argv[]) {
    
    // Starting mysh program with incorrect number of arguments
    if (argc > 2) {
        printf("%d\n", argc);
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
    
    while (1) {
        char* cmd = prompt();
        if (cmd[0] == 'q') {
            break;
        }
    }
    printf("quitting\n");
    return 0;
}