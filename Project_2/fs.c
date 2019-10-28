/*
  First Project for Operating systems.
  Modified by Matheus França and Nelson Trindade,
  ist191593 and ist193743, Group 22.
*/ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fs.h"

int obtainNewInumber(tecnicofs* fs, int hashcode) {
	//lock_function(1, lock[hashcode], rwlock[hashcode]);
	int newInumber = ++(fs->nextINumber);
	//unlock_function(lock[hashcode],rwlock[hashcode]);
	return newInumber;
}

tecnicofs* new_tecnicofs(int max){
	tecnicofs*fs = malloc(sizeof(tecnicofs));
	if (!fs) {
		perror("failed to allocate tecnicofs");
		exit(EXIT_FAILURE);
	}
	fs->hashMax=max;
	fs->bstRoot = (node**) malloc(sizeof(node)*max);
	for (int i=0;i < max;i++)
		fs->bstRoot[i] = NULL;
	fs->nextINumber = 0;
	return fs;
}

void free_tecnicofs(tecnicofs* fs){
	for (int i=0;i < fs->hashMax;i++)
		free_tree(fs->bstRoot[i]);
	free(fs->bstRoot);
	free(fs);
}

void create(tecnicofs* fs, char *name, int inumber, int hashcode){
	lock_function(1, lock[hashcode], rwlock[hashcode]);
	fs->bstRoot[hashcode] = insert(fs->bstRoot[hashcode], name, inumber);
	unlock_function(lock[hashcode],rwlock[hashcode]);
}

void delete(tecnicofs* fs, char *name, int hashcode){
	lock_function(1, lock[hashcode], rwlock[hashcode]);
	fs->bstRoot[hashcode] = remove_item(fs->bstRoot[hashcode], name);
	unlock_function(lock[hashcode],rwlock[hashcode]);
}

int lookup(tecnicofs* fs, char *name, int hashcode){
	lock_function(0, lock[hashcode], rwlock[hashcode]);
	node* searchNode = search(fs->bstRoot[hashcode], name);
	unlock_function(lock[hashcode],rwlock[hashcode]);
	if ( searchNode ) return searchNode->inumber;
	return 0;
}

void print_tecnicofs_tree(FILE * fp, tecnicofs *fs){
	for (int i=0; i < fs->hashMax ;i++)
		if (fs->bstRoot[i]!=NULL)
			print_tree(fp, fs->bstRoot[i]);
}
