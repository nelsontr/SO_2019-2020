/*
  First Project for Operating systems.
  File created by Matheus França and Nelson Trindade,
  ist191593 and ist193743, Group 22.
*/ 
#include "threads.h"

/*Global Variables (locks)*/
pthread_t tid[MAX_THREADS];
pthread_mutex_t **lock;
pthread_rwlock_t **rwlock;

/*Inicializes a specific lock*/
void lock_init(int max){
  #ifdef MUTEX  
    lock=malloc(sizeof(pthread_mutex_t)*max)
    for (int i=0;i<max;i++){
      pthread_mutex_init(&lock[i],NULL);   
    }
  #elif RWLOCK
    rwlock=malloc(sizeof(pthread_rwlock_t)*max)
    for (int i=0;i<max;i++){
      pthread_rwlock_init(&rwlock[i],NULL);
    }
  #endif
}

/*Destroys a specific lock*/
void lock_destroy(int max){
  #ifdef MUTEX
    for (int i=0;i<max;i++){
      pthread_mutex_destroy(&lock[i]);
    }
  #elif RWLOCK
    for (int i=0;i<max;i++){ 
      pthread_rwlock_destroy(&rwlock[i]);
    }
  #endif
}

/* Lock_function chooses between using Mutex_lock
or Rw/Wrlock depending on the executable */
void lock_function(int i, pthread_mutex_t* mutex, pthread_rwlock_t* rw){
	#ifdef MUTEX
		pthread_mutex_lock(&mutex);
	#elif RWLOCK
		if (i)	/*If it's !=0, than it locks for write*/
			pthread_rwlock_wrlock(&rw);
		else 	/*else, it locks for reading*/
			pthread_rwlock_rdlock(&rw);
	#endif
}

/* unlock_function chooses between using Mutex_unlock
or Rwlock_unlock depending on the executable */
void unlock_function(pthread_mutex_t* mutex, pthread_rwlock_t* rw){
	#ifdef MUTEX
		pthread_mutex_unlock(&mutex);
	#elif RWLOCK
		pthread_rwlock_unlock(&rw);
	#endif
}
