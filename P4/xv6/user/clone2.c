/* clone and play with the argument */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
volatile int arg = 55;
volatile int global = 1;

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void worker(void *arg_ptr);

int
main(int argc, char *argv[])
{
   ppid = getpid();
   void *stack = malloc(PGSIZE*2);
   assert(stack != NULL);
   if((uint)stack % PGSIZE)
     stack = stack + (4096 - (uint)stack % PGSIZE);
   
   printf(1, "Addr of arg in parent%p\n", &arg);
   int clone_pid = clone(worker, (void*)&arg, stack);
   assert(clone_pid > 0);
   assert(arg == 1);
   printf(1, "Global Var: %d\n", global);
   printf(1, "TEST PASSED\n");
   exit();
}

void
worker(void *arg_ptr) {
  printf(1, "Inside the worker\n");
  printf(1, "Worker Value of arg: %d\n",*(int *)arg_ptr);
  printf(1, "Addr of argument in worker: %p\n", arg_ptr); 
  int tmp = *(int*)arg_ptr;
   *(int*)arg_ptr = 1;
   assert(global == 1);
   global = tmp;
   printf(1, "Finishing the workder\n");
   exit();
}
