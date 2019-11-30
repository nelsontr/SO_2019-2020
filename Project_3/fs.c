/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */
/* Modified by Matheus and Nelson, group 22 */

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

tecnicofs* new_tecnicofs(){
	tecnicofs*fs = malloc(sizeof(tecnicofs));
	if (!fs) {
		perror("failed to allocate tecnicofs");
		exit(EXIT_FAILURE);
	}
	fs->nextINumber = 0;
	//sync_init(fs->bstLock);
	fs->bstRoot = NULL;
	return fs;
}

void free_tecnicofs(tecnicofs* fs){
	free_tree(fs->bstRoot);
	free(fs);
}

void create(tecnicofs* fs, char *name, int inumber, int flag){	/* if flag==0 then it locks*/
	//if (!flag) sync_wrlock(fs->bstLock);
	fs->bstRoot = insert(fs->bstRoot, name, inumber);
	//if (!flag) sync_unlock(fs->bstLock);
}

void delete(tecnicofs* fs, char *name, int flag){ /* if flag==0 then it locks*/
	//if (!flag) sync_wrlock(fs->bstLock);
	fs->bstRoot = remove_item(fs->bstRoot, name);
	//if (!flag) sync_unlock(fs->bstLock);
}

int lookup(tecnicofs* fs, char *name){
	//sync_rdlock(fs->bstLock);
	int inumber = -1;
	node* searchNode = search(fs->bstRoot, name);
	if ( searchNode ) {
		inumber = searchNode->inumber;
	}
	//sync_unlock(fs->bstLock);
	return inumber;
}

void renameFile(char* oldName,char* newName,tecnicofs *fs) {
	int	iNumberOld = lookup(fs,oldName);
	int iNumberNew = lookup(fs,newName);

	//sync_wrlock(fs->bstLock);
	if (!iNumberNew){
		if(iNumberOld){
			delete(fs,oldName,1);
			create(fs,newName,iNumberOld,1);
			//sync_unlock(fs->bstLock);
			return;
		}
		else {//sync_unlock(fs->bstLock);
		printf("%s doesn't existes!\n",oldName);return;} //Erro de nao existir
	}
	else {//sync_unlock(fs->bstLock);
	printf("%s already existes!\n",newName);return;} //Erro de ja existir			
}

void print_tecnicofs_tree(FILE * fp, tecnicofs *fs){
	if (fs->bstRoot!=NULL)
		print_tree(fp, fs->bstRoot);
}
