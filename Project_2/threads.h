/*
  First Project for Operating systems.
  File created by Matheus França and Nelson Trindade,
  ist191593 and ist193743, Group 22.
*/ 
#ifndef THREADS_H
#define THREADS_H
#include <pthread.h>    /*For creating threats*/


#define MAX_THREADS 100
/*Global Variables (locks)*/
pthread_t tid[MAX_THREADS];
pthread_mutex_t **lock;
pthread_rwlock_t **rwlock;

void lock_init(int max);
void lock_destroy(int max);
void lock_function(int i, pthread_mutex_t* mutex, pthread_rwlock_t* rw);
void unlock_function(pthread_mutex_t* mutex, pthread_rwlock_t* rw);

#endif