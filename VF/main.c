#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/time.h> 
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
  lock_function(1);
  if(numberCommands > 0){
    numberCommands--;
    unlock_function();
    return inputCommands[headQueue++];  
  }
  unlock_function();  
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



void* applyCommands(void *args){
  while(numberCommands > 0){
    const char* command = removeCommand();
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
        iNumber = obtainNewInumber(fs);
        create(fs, name, iNumber);
        break;
      case 'l':
        searchResult = lookup(fs, name);
        if(!searchResult)
          printf("%s not found\n", name);
        else
          printf("%s found with inumber %d\n", name, searchResult);
        break;
      case 'd':
        delete(fs, name);
        break;
      default: { /* error */
        fprintf(stderr, "Error: command to apply\n");
        exit(EXIT_FAILURE);
        }
    }
  }
  return NULL;
}

void aplly_command_main(int x){
    #if defined(MUTEX) || defined(RWLOCK)
      for (int i=0;i<x;i++)
        if (!pthread_create(&tid[i],NULL,applyCommands,NULL))
          fprintf(stderr, "Error: pthread_create failed to execute\n");
      for (int i=0;i<x;i++)
        if (!pthread_join(tid[i],NULL))
          fprintf(stderr, "Error: pthread_join failed to execute\n");
    #else
      applyCommands(NULL);
    #endif
}

/*Inicializes a specific lock*/
void lock_init(){
  #ifdef MUTEX
    pthread_mutex_init(&lock,NULL);
  #endif
  #ifdef RWLOCK
    pthread_rwlock_init(&rwlock,NULL);
  #endif
}

/*Destroys a specific lock*/
void lock_destroy(){
  #ifdef MUTEX
    pthread_mutex_destroy(&lock);
  #endif
  #ifdef RWLOCK
    pthread_rwlock_destroy(&rwlock);
  #endif
}


int main(int argc, char* argv[]) {
  parseArgs(argc, argv);
  struct timeval start, end; 
  
  lock_init();
  fs = new_tecnicofs();
  FILE *fout = fopen(argv[2],"w");
  gettimeofday(&start, NULL); /*Start clock*/
  processInput(argv[1]);

  aplly_command_main(atoi(argv[3]));
  print_tecnicofs_tree(fout, fs);

  gettimeofday(&end, NULL);   /*Ends clock*/
  fclose(fout);  
  lock_destroy();
  free_tecnicofs(fs);

  double time_taken = (end.tv_sec - start.tv_sec) * 1e6 
        + (end.tv_usec - start.tv_usec) * 1e-6; 
  printf("TecnicoFS completed in %.04f seconds.\n", time_taken);
  exit(EXIT_SUCCESS);
}
