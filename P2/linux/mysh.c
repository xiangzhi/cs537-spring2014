/* CS537 - Spring 2014 - Program 2
 * CREATED BY:  
 * Xiang Zhi Tan (xtan@cs.wisc.edu)
 * Roy Fang (fang@cs.wisc.edu)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <python2.6/Python.h>
#include <sys/wait.h>
#include <signal.h>
//max size is 514 because 512 character + '\n' + '\0'
#define MAX_SIZE 514

/* function prototypes */
//display when there is an error
void displayError(void);
//prompt the user in interaction mood
char* prompt(void);
//method to run execvp on fork
int runCommand(char**, int);
//parsing the inputs given by prompt or batch mode
int parseArgv(char*, char***);
//run the build in functions
int buildIn(char**, int);
//run python files
void execPython(char*, int, char**);
//run to redirect output
void redirectOutput(void);
//set whether to redirect output
void setOutput(void);
//close the output
void closeOutput(void);
//memoth to execute comamnd
int execute(char* cmd);

//added to prevent error
pid_t wait(int *stat_loc);

/* helper function */
//get a substring
char* substring(char* input, int start, int end);
//get the index of the character
int indexOf(char* input, char search_char);
//remove an element in the array
int arrayRemove(char*** _array, int size, int position);

//whether the output should be redirec
int redirectFlag;
//where to redireced to
char* redirectFileName;
//the filedescripter of the redirected file
int output_fd;
//whethere the current process should wait
int waitFlag;
//the input of cmd or batch input
char* input;


/*
 * usage of the program
 */
void usage(){
    displayError();
    exit(1);
}

/*
 * main process loop
 */
int main(int argc, char* argv[]) {
    
    // Starting mysh program with incorrect number of arguments
    if (argc < 1 || argc > 2) {
        usage();
    }

    //allocate the input
    input = malloc(MAX_SIZE);

    //batch mode
    if(argc == 2 ){
        FILE *input_fd;
        char* inputName = argv[1];
        // open the input file
        input_fd = fopen(inputName, "r");
        // check whther the input file has open correctly
        if (input_fd == NULL){
		
            displayError();
            exit(1);
        }
        //get the next line
        input = fgets(input, MAX_SIZE, input_fd);
        while(input != NULL) {
            //output the file's command
            write(STDOUT_FILENO, input, strlen(input));
            char* new_line = strchr(input, '\n');
	
            //means newline is not in the string
            //which implies the string is longer than 512
            if(new_line == NULL){
	           write(STDOUT_FILENO, "\n", 1);
	           int c;
	           while ((c = getc(input_fd)) != '\n' && c != EOF);
                displayError();
	            input = fgets(input, MAX_SIZE, input_fd);
                continue;
            }
	       //find first instance of '\n'
            if(input[strlen(input)-1] == '\n'){
                input[strlen(input)-1] = '\0';
            }
            //exectute the input
            execute(input);
            //get the next line
            input = fgets(input, MAX_SIZE, input_fd);
        }
        //finish execution done;
        return 0;
    }
    else{
        while (1) {
            //get the next command
            char* cmd = prompt();
            //check whether prompt was successful
            if(cmd == NULL){
                //continue;
            }
            else{
                //execute the input
                execute(cmd);
            }
        }
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
    //output msg
    char* msg = "mysh> ";
    write(STDOUT_FILENO, msg, strlen(msg));
    //get the input from STDIN
    input = fgets(input, MAX_SIZE, stdin);

    //check whether fgets is successful
    if(input == NULL){
        //check whether because it just reach end of file
        if(feof(stdin) > 0){
            exit(0);
        }

        displayError();
        return NULL;
    }
    //find first instance of '\n'
    char* new_line = strchr(input, '\n');

    //means newline is not in the string
    //which implies the string is longer than 512
    if(strlen(input) == MAX_SIZE && new_line == NULL){
        char* _EOF = strchr(input, EOF);
        if(_EOF == NULL){
        	int c;
        	while ((c = getchar()) != '\n' && c != EOF);
                displayError();
                return NULL;
        }
    }
    //replace new_line with terminate string
    if(new_line != NULL){
        *new_line = '\0';
    }
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
 * Excuting the command
 */
int execute(char* cmd){
    //reset flags
    waitFlag = 0;
    redirectFlag = -1;

    //parse inputs;
    char** exec_args;
    int num_argv = parseArgv(cmd, &exec_args);
    //check whether parsing was successful
    if(num_argv == -1){
        return 1;
    }
    //change the STDIO if needed
    setOutput();
    //try running it as build in function
    int check = buildIn(exec_args, num_argv);
    //see whether buildin functon run succesfully or exited with error
    if(check == 0 || check == 2){
        closeOutput();
        free(exec_args);
        return 1;
    }
    //try running as fork()
    check = runCommand(exec_args, num_argv);
    //free(exec_args);
    return 0;
}

/*
 * function that deal with execution of function
 */
int runCommand(char** exec_args, int num_argv){

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
            //write(STDOUT_FILENO, "test", strlen("test"));
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
        if(waitFlag == 0){
            //write(STDOUT_FILENO, "wait", strlen("wait"));
            int wc = wait(NULL);
            if(wc == -1){
                displayError();
                exit(1);
            }
        }
        //write(STDOUT_FILENO, "here?", strlen("here?"));
        return 0;
    }
}

/*
 * Parsing the input
 */
int parseArgv(char* input, char*** exec_args){

    int i;

    //count the number of arguments in the list;
    char* pointer = (char*) malloc( strlen(input) * sizeof(char) );
    pointer = strncpy(pointer, input, strlen(input) * sizeof(char) );
    //try copying the input to other stuff
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


    //check for waiting
    //check if its by itself
    if( strcmp(list[index-1],"&") == 0 && 
        strlen(list[index-1]) == 1){
        waitFlag = 1;
        index--;
    }

    //check whether if its with a command but still at the end
    if(list[index-1][strlen(list[index-1]) -1 ] == '&'){
        waitFlag = 1;
        list[index-1][strlen(list[index-1]) -1 ] = '\0';
    }

    //check for the redirect flag
    for(i = 0; i < index; i++){
        char* word = list[i];
        int char_index = indexOf(word, '>');
        if(char_index != -1){
            if(redirectFlag == 1){
                displayError();
                return -1;
            }

            //case zero the > is by itself
            if(strlen(word) == 1){
                if( (i + 1) != (index - 1) ){
                    displayError();
                    return -1;
                }
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

                if(i != index - 1){
                    displayError();
                    return -1;
                }
                //remove the fileName from the list
                arrayRemove(&list, index, i);
                index--;
                i--;
            }
            //second case,  where < is at the end -> ls> text.txt
            else if(char_index == (strlen(word) - 1) ){

                //if the > exsist in the end without a file Name
                if((i + 1) == index || (i + 1) != (index - 1)){
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
                if( (i + 1) != index ){
                    displayError();
                    return -1;
                }
                list[i] = substring(word, 0, char_index);

                redirectFileName = substring(word, char_index + 1, strlen(word));
            }
            redirectFlag = 1;
            //printf("filename:%s\n", redirectFileName);
        }
    }

    //debuging to see what command is left in the system;
    /*
    for(i= 0; i < index; i++){
        printf("\n%d: %s\n",i, list[i]);
    }
    */

    //the last argument is a NULL
    list[index] = NULL;
    
    free(pointer);
    *exec_args = list;
    return index;
}

/*
 * See whether they are build in functions
 */
int buildIn(char** exec_args, int num_argv){
    char* command = exec_args[0];

    //check whether we reach the quit phase
    if (strcmp(command, "exit") == 0) {
	if (num_argv != 1) {
		displayError();
		return 0;
	}
        free(exec_args);
        exit(0);
    }

    //whether is wait or not
    if (strcmp(command, "wait") == 0) {
	if (num_argv != 1) {
		displayError();
		return 0;
	}
        while(wait(NULL) > 0);
        return 0;
    }
    
    // Check for .py substring
    if (strstr(command, ".py") != NULL) {
        execPython(command, num_argv, exec_args);
        return 0;
    }

    
    //check for pwd
    if (strcmp(command, "pwd") == 0) {
	if (num_argv != 1) {
		displayError();
		return 0;
	}
        char* path = getcwd(NULL, 0);;
        write(output_fd, path, strlen(path));
        write(output_fd, "\n", 1);
        free(path);
        return 0;
    }
    //check for cd
    if (strcmp(command, "cd") == 0){
        if(num_argv > 2 || num_argv < 1){
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

/*
 * See what output it should be
 */
void setOutput(void){
    if(redirectFlag == 1){
        output_fd = open(redirectFileName, 
            O_RDWR | O_TRUNC | O_CREAT, S_IRWXU);
    }
    else{
        output_fd = STDOUT_FILENO;
    }
}

/*
 * see whether should close STDIN
 */
void closeOutput(void){
    if(redirectFlag == 1){
        close(output_fd);
        output_fd = STDIN_FILENO;
    }
}

/*
 * run to redirect output
 */
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

/*
 * to run python 
 */
void execPython(char* fileName, int argc, char* argv[]) {

    //call fork
    int rc  = fork();
    //check whether fork was succesful
    if(rc < 0){
        displayError();
        exit(1);
    }

    if(rc == 0){
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
        exit(0);
    }
    else{
        int wc = wait(NULL);
        if(wc == -1){
            displayError();
            exit(1);
        }
        return;
    }
}

/* Copied from private made libraries*/

/* remove items at a specific location in an array */
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

/*
 * Return a safe substring of the input with the start and end defined
 */
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

/*
 * Find the first occurance of the char in the string
 */
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
