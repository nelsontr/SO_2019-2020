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
    node** bstRoot;
    int nextINumber;
    int hashMax;
      
} tecnicofs;

int obtainNewInumber(tecnicofs* fs, int hashcode);
tecnicofs* new_tecnicofs(int max);
void free_tecnicofs(tecnicofs* fs);
void create(tecnicofs* fs, char *name, int inumber, int hashcode);
void delete(tecnicofs* fs, char *name, int hashcode);
int lookup(tecnicofs* fs, char *name, int hashcode);
void print_tecnicofs_tree(FILE * fp, tecnicofs *fs);

#endif /* FS_H */
