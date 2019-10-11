/*
  First Project for Operating systems.
  Modified by Matheus França and Nelson Trindade,
  ist191593 and ist193743, Group 22.
*/ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fs.h"

int obtainNewInumber(tecnicofs* fs) {
	lock_function(1, lock, rwlock);
	int newInumber = ++(fs->nextINumber);
	unlock_function(lock,rwlock);
	return newInumber;
}

tecnicofs* new_tecnicofs(){
	tecnicofs*fs = malloc(sizeof(tecnicofs));
	if (!fs) {
		perror("failed to allocate tecnicofs");
		exit(EXIT_FAILURE);
	}
	fs->bstRoot = NULL;
	fs->nextINumber = 0;
	return fs;
}

void free_tecnicofs(tecnicofs* fs){
	free_tree(fs->bstRoot);
	free(fs);
}

void create(tecnicofs* fs, char *name, int inumber){
	lock_function(1, lock, rwlock);
	fs->bstRoot = insert(fs->bstRoot, name, inumber);
	unlock_function(lock,rwlock);
}

void delete(tecnicofs* fs, char *name){
	lock_function(1, lock, rwlock);
	fs->bstRoot = remove_item(fs->bstRoot, name);
	unlock_function(lock,rwlock);
}

int lookup(tecnicofs* fs, char *name){
	lock_function(0, lock, rwlock);
	node* searchNode = search(fs->bstRoot, name);
	unlock_function(lock,rwlock);
	if ( searchNode ) return searchNode->inumber;
	return 0;
}

void print_tecnicofs_tree(FILE * fp, tecnicofs *fs){
	print_tree(fp, fs->bstRoot);
}
