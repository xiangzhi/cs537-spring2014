#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char* argv[]){

  //fork() creates 2 copies of the process.

  int rc  = fork();
  //both calls continue at the same place.
  if(rc < 0){
    perror("fork");
    exit(1);
  }

  //this is the child
  if(rc == 0){
    printf("I'm the child, r:%d, pid: %d\n", rc,  (int) getpid() );

    //execvp() call to replace the whole address space.
    
    //first prepare args for execvp
    char* exec_args[4];

    exec_args[0] = "ls";
    exec_args[1] = "-l";
    exec_args[2] = "-a";
    //the last argument must be NULL to terminate list
    exec_args[3] = NULL;  

    int close_rc = close(STDOUT_FILENO);
    if(close_rc < 0){
      perror("close");
      exit(1);
    }

    //Open a new file
    int fd = open("redirected_output.txt",O_CREAT|  O_RDWR | O_TRUNC, S_IRWXU);
    if(fd  < 0){
      perror("open");
      exit(1);
    }

    printf("fd = %d\n", fd);

    //call execvp
    //execvp should have never retunr
    execvp("ls", exec_args);

    /* execvp will return if there is an error, can be used to be set as a error handler for invalid inputs*/

    printf("You'll never see this\n");

  }
  //this is the parent
  else{
    //wait untill anyone of the children finishs
    int wc = wait(NULL);

    printf("I'm parent, r:%d,  pid: %d\n",rc,  (int) getpid() );
  }


}