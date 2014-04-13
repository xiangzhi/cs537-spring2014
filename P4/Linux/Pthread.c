//wrapper function for pthread
#include "Pthread.h"

int Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void* routine, void *arg){
    return pthread_create(thread, attr, routine,arg);
}

int Pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr){
    return pthread_mutex_init(mutex, attr);
}

void Pthread_mutex_lock(pthread_mutex_t *mutex){
    if(pthread_mutex_lock(mutex) != 0){
        ErrorDisplay("pthread_mutex_lock");
    }
}

void Pthread_mutex_unlock(pthread_mutex_t *mutex){
    if(pthread_mutex_unlock(mutex) != 0){
        ErrorDisplay("pthread_mutex_unlock");
    }
}

int Pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr){
    return pthread_cond_init(cond, attr);
}

void Pthread_cond_signal(pthread_cond_t *cond){
    if(pthread_cond_signal(cond) != 0){
        ErrorDisplay("pthread_cond_singal");
    }
}
//Atomically unlock the specified mutex and block on a condition.
void Pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex){
    if(pthread_cond_wait(cond, mutex) != 0){
        ErrorDisplay("pthread_cond_wait");
    }
}

void ErrorDisplay(char* name){
    fprintf(stderr, "%s failed\n", name);
    exit(1);
}
