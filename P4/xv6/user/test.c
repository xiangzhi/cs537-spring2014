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
  assert(0,lock_init(&lock));
  
  lock_t* invalid = NULL;
  assert(-1,lock_init(invalid));
  assert(0,lock_acquire(&lock));
  assert(0,lock_release(&lock));
  int i;
  for(i = 0; i < 10; i++){
    assert(0,thread_create(worker,(void*)&i));
  }
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
