// Do not modify this file. It will be replaced by the grading scripts
// when checking your project.

#include "types.h"
#include "stat.h"
#include "user.h"

void assert(int expect,int returnV);
void worker(void* ptr);

lock_t lock;
int o = 0;

int
main(int argc, char *argv[])
{
  if(argc != 3){
    printf(1,"usage: test <threads> <loop>\n");
    exit();
  }
  assert(0,lock_init(&lock));
  o = atoi(argv[2]);
  int threadCount = atoi(argv[1]);
  lock_t* invalid = NULL;
  assert(-1,lock_init(invalid));
  assert(0,lock_acquire(&lock));
  assert(0,lock_release(&lock));
  int i;
  for(i = 0; i < threadCount; i++){
    int x = thread_create(worker,(void*)&i);
    if( x < 0){
       printf(1,"error: thread less than 0");
       exit();
    }
  }
  assert(0,thread_join());
  exit();
}

void worker(void* ptr){
  int value = *((int*)ptr);
  while(o < 1000){
    assert(0,lock_acquire(&lock));
    if(o < 1000){
	break;
    }
    printf(1,"Hello From Thread %d\n",value);
    o++;
    assert(0,lock_release(&lock));
  }
}

void 
assert(int expect, int returnV){
  if(expect != returnV){
    printf(1,"expected:%d got:%d",expect,returnV);
  }
}