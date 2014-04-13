//wrapper function for pthread
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

int Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void* routine, void *arg);
void Pthread_mutex_lock(pthread_mutex_t *mutex);
int Pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
void Pthread_mutex_unlock(pthread_mutex_t *mutex);
int  Pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
void Pthread_cond_signal(pthread_cond_t *cond);
//Atomically unlock the specified mutex and block on a condition.
void Pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

void ErrorDisplay(char* name);


