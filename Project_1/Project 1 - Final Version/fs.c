/*
  First Project for Operating systems.
  Modified by Matheus França and Nelson Trindade,
  ist191593 and ist193743, Group 22.
*/ 
#include "fs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

int obtainNewInumber(tecnicofs* fs) {
	lock_function(1, lock, rwlock);
	int newInumber = ++(fs->nextINumber);
	unlock_function(lock,rwlock);
	return newInumber;
}

tecnicofs* new_tecnicofs(){
	tecnicofs*fs = malloc(sizeof(tecnicofs));
	if (!fs) {
		perror("failed to allocate tecnicofs");
		exit(EXIT_FAILURE);
	}
	fs->bstRoot = NULL;
	fs->nextINumber = 0;
	return fs;
}

void free_tecnicofs(tecnicofs* fs){
	free_tree(fs->bstRoot);
	free(fs);
}

void create(tecnicofs* fs, char *name, int inumber){
	lock_function(1, lock, rwlock);
	fs->bstRoot = insert(fs->bstRoot, name, inumber);
	unlock_function(lock,rwlock);
}

void delete(tecnicofs* fs, char *name){
	lock_function(1, lock, rwlock);
	fs->bstRoot = remove_item(fs->bstRoot, name);
	unlock_function(lock,rwlock);
}

int lookup(tecnicofs* fs, char *name){
	lock_function(0, lock, rwlock);
	node* searchNode = search(fs->bstRoot, name);
	unlock_function(lock,rwlock);
	if ( searchNode ) return searchNode->inumber;
	return 0;
}

void print_tecnicofs_tree(FILE * fp, tecnicofs *fs){
	print_tree(fp, fs->bstRoot);
}
