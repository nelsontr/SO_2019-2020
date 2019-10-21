/*
  First Project for Operating systems.
  File created by Matheus França and Nelson Trindade,
  ist191593 and ist193743, Group 22.
*/ 
#include "threads.h"

/*Inicializes a specific lock*/
void lock_init(){
  #ifdef MUTEX
    pthread_mutex_init(&lock,NULL);
    pthread_mutex_init(&lock_commands,NULL);    
  #elif RWLOCK
    pthread_rwlock_init(&rwlock,NULL);
    pthread_rwlock_init(&rwlock_commands,NULL);
  #endif
}

/*Destroys a specific lock*/
void lock_destroy(){
  #ifdef MUTEX
    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&lock_commands);
  #elif RWLOCK
    pthread_rwlock_destroy(&rwlock);
    pthread_rwlock_destroy(&rwlock_commands);
  #endif
}

/* Lock_function chooses between using Mutex_lock
or Rw/Wrlock depending on the executable */
void lock_function(int i, pthread_mutex_t mutex, pthread_rwlock_t rw){
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
void unlock_function(pthread_mutex_t mutex, pthread_rwlock_t rw){
	#ifdef MUTEX
		pthread_mutex_unlock(&mutex);
	#elif RWLOCK
		pthread_rwlock_unlock(&rw);
	#endif
}
