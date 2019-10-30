/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */

#include "fs.h"
#include "lib/bst.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sync.h"



int obtainNewInumber(tecnicofs* fs) {
	int newInumber = ++(fs->nextINumber);
	return newInumber;
}

tecnicofs* new_tecnicofs(int max){
	tecnicofs*fs = malloc(sizeof(tecnicofs));
	if (!fs) {
		perror("failed to allocate tecnicofs");
		exit(EXIT_FAILURE);
	}
	fs->hashMax=max;
	fs->nextINumber = 0;
	fs->bstRoot = (node **) malloc(sizeof(node)*max);
	fs->bstLock = (syncMech *) malloc(sizeof(syncMech)*max);
	for (int i=0;i < max;i++){
		fs->bstRoot[i] = NULL;
		sync_init(&(fs->bstLock[i]));
	}		
	return fs;
}

void free_tecnicofs(tecnicofs* fs){
	for (int i=0;i < fs->hashMax;i++){
		free_tree(fs->bstRoot[i]);
		sync_destroy(&(fs->bstLock[i]));
	}
	free(fs->bstRoot);
	free(fs);
}

void create(tecnicofs* fs, char *name, int inumber, int hashcode){
	sync_wrlock(&(fs->bstLock[hashcode]));
	fs->bstRoot[hashcode] = insert(fs->bstRoot[hashcode], name, inumber);
	sync_unlock(&(fs->bstLock[hashcode]));
}

void delete(tecnicofs* fs, char *name, int hashcode){
	sync_wrlock(&(fs->bstLock[hashcode]));
	fs->bstRoot[hashcode] = remove_item(fs->bstRoot[hashcode], name);
	sync_unlock(&(fs->bstLock[hashcode]));
}

int lookup(tecnicofs* fs, char *name, int hashcode){
	sync_rdlock(&(fs->bstLock[hashcode]));
	int inumber = 0;
	node* searchNode = search(fs->bstRoot[hashcode], name);
	if ( searchNode ) {
		inumber = searchNode->inumber;
	}
	sync_unlock(&(fs->bstLock[hashcode]));
	return inumber;
}

void print_tecnicofs_tree(FILE * fp, tecnicofs *fs){
	for (int i=0; i < fs->hashMax ;i++)
		if (fs->bstRoot[i]!=NULL)
			print_tree(fp, fs->bstRoot[i]);
}
