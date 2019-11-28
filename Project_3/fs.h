/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */
/* Modified by Matheus and Nelson, group 22 */

#ifndef FS_H
#define FS_H
#include "sync.h"
#include "lib/bst.h"
#include "lib/hash.h"
#include "lib/inodes.h"

typedef struct tecnicofs {
    node* bstRoot;
    int nextINumber;
    int hashMax;
    syncMech bstLock;
    inode_t* inodeTable;
} tecnicofs;

int obtainNewInumber(tecnicofs* fs);
tecnicofs* new_tecnicofs();
void free_tecnicofs(tecnicofs* fs);
void create(tecnicofs* fs, char *name, int inumber, int flag);
void delete(tecnicofs* fs, char *name, int flag);
int lookup(tecnicofs* fs, char *name);
void renameFile(char* oldName,char* newName,tecnicofs *fs);
void print_tecnicofs_tree(FILE * fp, tecnicofs *fs);

#endif /* FS_H */
