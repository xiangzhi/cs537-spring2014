#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

int lock_init(lock_t *lock) {
  if (lock == NULL) {
    return -1;
  }
  lock->locked = 0;
  return 0;
}

int lock_acquire(lock_t *lock) {
  if (lock == NULL) {
    return -1;
  }

  while (xchg(&(lock->locked), 1) == 1); //spin
  return 0;
}

int lock_release(lock_t *lock) {
  xchg(&(lock->locked), 0);
  return 0;
}

int thread_create(void (*fcn) (void*), void *arg) {
  void * stack;
  uint size = 4096;
  stack = malloc(size);
  return clone(fcn, arg, stack);
} 

int thread_join() {
  
  void * stack;
  return join(&stack);
}

int cv_wait(cond_t *cv, lock_t *lock) {
	lock_release(lock);
	//Cond_wait sys call
	lock_acquire(lock);
	
	return 0;
}

int cv_signal(cond_t *cv){
	//cond_signal sys call
	return 0;
}