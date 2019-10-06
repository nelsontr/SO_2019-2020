#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include "fs.h"

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

int numberThreads = 0;
tecnicofs* fs;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;
/*pthread's*/
pthread_t tid[MAX_INPUT_SIZE];
pthread_mutex_t lock;
pthread_rwlock_t rwlock;

static void displayUsage (const char* appName){
  printf("Usage: %s\n", appName);
  exit(EXIT_FAILURE);
}

static void parseArgs (long argc, char* const argv[]){
  if (argc != 4) {
    fprintf(stderr, "Invalid format:\n");
    displayUsage(argv[0]);
  }
}

int insertCommand(char* data) {
  if(numberCommands != MAX_COMMANDS) {
    strcpy(inputCommands[numberCommands++], data);
    return 1;
  }
  return 0;
}

char* removeCommand() {
  if(numberCommands > 0){
    numberCommands--;
    return inputCommands[headQueue++];  
  }
  return NULL;
}

void errorParse(){
  fprintf(stderr, "Error: command invalid\n");
  //exit(EXIT_FAILURE);
}

void processInput(char* f_in){
  FILE *fin = fopen(f_in, "r");
  char line[MAX_INPUT_SIZE];

  while (fgets(line, sizeof(line)/sizeof(char), fin)) {
    char token;
    char name[MAX_INPUT_SIZE];

    int numTokens = sscanf(line, "%c %s", &token, name);

    /* perform minimal validation */
    if (numTokens < 1) {
      continue;
    }
    switch (token) {
      case 'c':
      case 'l':
      case 'd':
        if(numTokens != 2)
          errorParse();
        if(insertCommand(line))
          break;
        return;
      case '#':
        break;
      default: { /* error */
        errorParse();
      }
    }
  }
  fclose(fin);
}

void lock_function(int i){
    #ifdef MUTEX
        pthread_mutex_lock(&lock);
    #endif
    #ifdef RWLOCK
        if (i){
            pthread_rwlock_wrlock(&rwlock);
        }
        else 
            pthread_rwlock_rdlock(&rwlock);
    #endif
}

void unlock_function(){
    #ifdef MUTEX
        pthread_mutex_unlock(&lock);
    #endif
    #ifdef RWLOCK
        pthread_rwlock_unlock(&rwlock);
    #endif
}

void* applyCommands(void *args){
  FILE *fout = (FILE *) args;
  while(numberCommands > 0){
    lock_function(1);
    const char* command = removeCommand();
    unlock_function();
    if (command == NULL){
      continue;
    }

    char token;
    char name[MAX_INPUT_SIZE];
    int numTokens = sscanf(command, "%c %s", &token, name);
    if (numTokens != 2) {
      fprintf(stderr, "Error: invalid command in Queue\n");
      exit(EXIT_FAILURE);
    }

    int searchResult;
    int iNumber;
    switch (token) {
      case 'c':
        lock_function(1);
        iNumber = obtainNewInumber(fs);
        unlock_function();
        lock_function(0);
        create(fs, name, iNumber);
        unlock_function();
        break;
      case 'l':
        lock_function(0);
        searchResult = lookup(fs, name);
        unlock_function();
        if(!searchResult)
          printf("%s not found\n", name);
        else
          printf("%s found with inumber %d\n", name, searchResult);
        unlock_function();
        break;
      case 'd':
        lock_function(1);
        delete(fs, name);
        unlock_function();
        break;
      default: { /* error */
        fprintf(stderr, "Error: command to apply\n");
        exit(EXIT_FAILURE);
        }
    }
  }
  return NULL;
}

void aplly_command_main(FILE* fout,int x){
    #if defined(MUTEX) || defined(RWLOCK)
    for (int i=0;i<x;i++)
        pthread_create(&tid[i],0,applyCommands,fout);
    for (int i=0;i<x;i++)
        pthread_join(&tid[i],NULL);
    #else
        applyCommands(fout);
    #endif
}

void lock_init(){
  #ifdef MUTEX
        pthread_mutex_init(&lock,NULL);
  #endif
  #ifdef RWLOCK
      pthread_rwlock_init(&rwlock,NULL);
  #endif
}

int main(int argc, char* argv[]) {
  parseArgs(argc, argv);
  FILE *fout = fopen(argv[2],"w");
  lock_init();

  fs = new_tecnicofs();
  processInput(argv[1]);

  printf("%d", atoi(argv[3]));

  aplly_command_main(fout,atoi(argv[3]));
  print_tecnicofs_tree(fout, fs);

  free_tecnicofs(fs);
  fclose(fout);
  exit(EXIT_SUCCESS);
}
