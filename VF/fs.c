#include "fs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Lock_function chooses between using Mutex_lock
or Rw/Wrlock depending on the executable */
void lock_function(int i){
	#ifdef MUTEX
		pthread_mutex_lock(&lock[0]);
	#endif
	#ifdef RWLOCK
		if (i)	/*If it's !=0, than it locks for write*/
			pthread_rwlock_wrlock(&rwlock);
		else 	/*else, it locks for reading*/
			pthread_rwlock_rdlock(&rwlock);
	#endif
}

/* unlock_function chooses between using Mutex_unlock
or Rwlock_unlock depending on the executable */
void unlock_function(){
	#ifdef MUTEX
		pthread_mutex_unlock(&lock[0]);
	#endif
	#ifdef RWLOCK
		pthread_rwlock_unlock(&rwlock);
	#endif
}

int obtainNewInumber(tecnicofs* fs) {
	lock_function(1);
	int newInumber = ++(fs->nextINumber);
	unlock_function();
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
	lock_function(1);
	fs->bstRoot = insert(fs->bstRoot, name, inumber);
	unlock_function();
}

void delete(tecnicofs* fs, char *name){
	lock_function(1);
	fs->bstRoot = remove_item(fs->bstRoot, name);
	unlock_function();
}

int lookup(tecnicofs* fs, char *name){
	lock_function(0);
	node* searchNode = search(fs->bstRoot, name);
	unlock_function();
	if ( searchNode ) return searchNode->inumber;
	return 0;
}

void print_tecnicofs_tree(FILE * fp, tecnicofs *fs){
	print_tree(fp, fs->bstRoot);
}
