/*
  First Project for Operating systems.
  Modified by Matheus França and Nelson Trindade,
  ist191593 and ist193743, Group 22.
*/ 
#ifndef FS_H
#define FS_H
#include "lib/bst.h"
#include <pthread.h>    /*For creating threats*/


typedef struct tecnicofs {
    node* bstRoot;
    int nextINumber;
} tecnicofs;

/*Global Variables (locks)*/
pthread_mutex_t lock_commands,lock;
pthread_rwlock_t rwlock_commands,rwlock;

void lock_function(int i, pthread_mutex_t mutex, pthread_rwlock_t rw);
void unlock_function(pthread_mutex_t mutex, pthread_rwlock_t rw);
int obtainNewInumber(tecnicofs* fs);
tecnicofs* new_tecnicofs();
void free_tecnicofs(tecnicofs* fs);
void create(tecnicofs* fs, char *name, int inumber);
void delete(tecnicofs* fs, char *name);
int lookup(tecnicofs* fs, char *name);
void print_tecnicofs_tree(FILE * fp, tecnicofs *fs);

#endif /* FS_H */
