#ifndef THREADS_H
#define THREADS_H
#include "fs.h"
#include <pthread.h>    /*For creating threats*/


#define MAX_THREADS 100
/*Global Variables (locks)*/
pthread_t tid[MAX_THREADS];
pthread_mutex_t lock_commands,lock;
pthread_rwlock_t rwlock_commands,rwlock;

void lock_init();
void lock_destroy();
void lock_function(int i, pthread_mutex_t mutex, pthread_rwlock_t rw);
void unlock_function(pthread_mutex_t mutex, pthread_rwlock_t rw);


#endif