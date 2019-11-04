/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */

#ifndef FS_H
#define FS_H
#include "lib/bst.h"
#include "lib/hash.h"
#include "sync.h"

typedef struct tecnicofs {
    node** bstRoot;
    int nextINumber;
    int hashMax;
    syncMech* bstLock;
} tecnicofs;

int obtainNewInumber(tecnicofs* fs);
tecnicofs* new_tecnicofs(int max);
void free_tecnicofs(tecnicofs* fs);
void create(tecnicofs* fs, char *name, int inumber, int hashcode);
void delete(tecnicofs* fs, char *name, int hashcode);
int lookup(tecnicofs* fs, char *name, int hashcode);
void renameFile(char* oldName,char* newName,int hashCode,tecnicofs *fs,int hashMax);
void print_tecnicofs_tree(FILE * fp, tecnicofs *fs);

#endif /* FS_H */
