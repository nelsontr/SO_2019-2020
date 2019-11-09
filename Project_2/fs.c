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
	free(fs->bstLock);
	free(fs);
}

void create(tecnicofs* fs, char *name, int inumber, int flag){
	int hashcode=hash(name,fs->hashMax);
	if (!flag) sync_wrlock(&(fs->bstLock[hashcode]));
	fs->bstRoot[hashcode] = insert(fs->bstRoot[hashcode], name, inumber);
	if (!flag) sync_unlock(&(fs->bstLock[hashcode]));
}

void delete(tecnicofs* fs, char *name, int flag){
	int hashcode=hash(name,fs->hashMax);
	if (!flag) sync_wrlock(&(fs->bstLock[hashcode]));
	fs->bstRoot[hashcode] = remove_item(fs->bstRoot[hashcode], name);
	if (!flag) sync_unlock(&(fs->bstLock[hashcode]));
}

int lookup(tecnicofs* fs, char *name){
	int hashcode=hash(name,fs->hashMax);
	sync_rdlock(&(fs->bstLock[hashcode]));
	int inumber = 0;
	node* searchNode = search(fs->bstRoot[hashcode], name);
	if ( searchNode ) {
		inumber = searchNode->inumber;
	}
	sync_unlock(&(fs->bstLock[hashcode]));
	return inumber;
}

void renameFile(char* oldName,char* newName,tecnicofs *fs) {		//FALTA LOCKS
	int locknew = hash(newName,fs->hashMax);
	int lockold = hash(oldName, fs->hashMax);
	int	iNumberOld = lookup(fs,oldName);
	int iNumberNew = lookup(fs,newName);
	 
	if (!iNumberNew) {	// Se o inumber do novo nome retornar igual a 0
		if (locknew==lockold){
			sync_wrlock(&(fs->bstLock[locknew]));
			delete(fs,oldName,1);
			create(fs,newName,iNumberOld,1);
			sync_unlock(&(fs->bstLock[locknew]));
			return;
		}	
		//NOS TESTES, APAERECE AS VEZES F E AO MESMO TEMPO FA, CORRIGIR
		else
			while(1){
				sync_wrlock(&(fs->bstLock[locknew]));
				int err = syncMech_try_lock(&(fs->bstLock[lockold]));
				if (!err){
					delete(fs,oldName,1);
					create(fs,newName,iNumberOld,1);
					sync_unlock(&(fs->bstLock[lockold]));
					sync_unlock(&(fs->bstLock[locknew]));
					return;
				}
				sync_unlock(&(fs->bstLock[locknew]));
			}
	}
	else {puts("ALO");return;} //Erro de ja existir		
}
/*void renameFile(char* oldName,char* newName,tecnicofs *fs) {
	int hashNew = hash(newName,fs->hashMax);
	int hashOld = hash(oldName, fs->hashMax);
	while (!lookup(fs,newName)) {
		int iNumber = lookup(fs,oldName);
		if (lookup(fs,oldName)) {
			if (!lookup(fs,newName)) {
				if (hashNew == hashOld) {
					if (!syncMech_try_lock(&(fs->bstLock[hashNew]))) {
						delete(fs,oldName,1);
						create(fs,newName,iNumber,1);
						sync_unlock(&(fs->bstLock[hashNew]));
						return;
					} else {
						sync_unlock(&(fs->bstLock[hashNew]));
					}
				} else {
					if (!syncMech_try_lock(&(fs->bstLock[hashOld]))) {
						delete(fs,oldName,1);
						sync_unlock(&(fs->bstLock[hashOld]));
					}
					if (!syncMech_try_lock(&(fs->bstLock[hashNew]))) {
						create(fs,newName,iNumber,1);
						sync_unlock(&(fs->bstLock[hashNew]));
					}
				}
			}
		}
	}	
	return;
}*/

void print_tecnicofs_tree(FILE * fp, tecnicofs *fs){
	for (int i=0; i < fs->hashMax ;i++)
		if (fs->bstRoot[i]!=NULL)
			print_tree(fp, fs->bstRoot[i]);
}
