#ifndef FS_H
#define FS_H
#include "lib/bst.h"
#include <pthread.h>    /*For creating threats*/


typedef struct tecnicofs {
    node* bstRoot;
    int nextINumber;
} tecnicofs;

/*Global Variables (locks)*/
pthread_mutex_t lock[2];
pthread_rwlock_t rwlock;

void lock_function(int i);
void unlock_function();
int obtainNewInumber(tecnicofs* fs);
tecnicofs* new_tecnicofs();
void free_tecnicofs(tecnicofs* fs);
void create(tecnicofs* fs, char *name, int inumber);
void delete(tecnicofs* fs, char *name);
int lookup(tecnicofs* fs, char *name);
void print_tecnicofs_tree(FILE * fp, tecnicofs *fs);

#endif /* FS_H */
