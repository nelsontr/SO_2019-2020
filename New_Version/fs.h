/*
  First Project for Operating systems.
  Modified by Matheus França and Nelson Trindade,
  ist191593 and ist193743, Group 22.
*/ 
#ifndef FS_H
#define FS_H
#include "lib/bst.h"
#include "threads.h"

typedef struct tecnicofs {
    node* bstRoot;
    int nextINumber;
} tecnicofs;

int obtainNewInumber(tecnicofs* fs);
tecnicofs* new_tecnicofs();
void free_tecnicofs(tecnicofs* fs);
void create(tecnicofs* fs, char *name, int inumber);
void delete(tecnicofs* fs, char *name);
int lookup(tecnicofs* fs, char *name);
void print_tecnicofs_tree(FILE * fp, tecnicofs *fs);

#endif /* FS_H */
