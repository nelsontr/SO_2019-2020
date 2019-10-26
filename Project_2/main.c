/*
  First Project for Operating systems.
  Modified by Matheus França and Nelson Trindade,
  ist191593 and ist193743, Group 22.
*/ 
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include "fs.h"
#include "threads.h"
#include "lib/hash.h"

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100


char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;
int MAX=0;
tecnicofs* fs;

static void displayUsage (const char* appName){
  printf("Usage: %s\n", appName);
  exit(EXIT_FAILURE);
}

static void parseArgs (long argc, char* const argv[]){
  if (argc != 5) {
    fprintf(stderr, "Invalid format:\n");
    displayUsage(argv[0]);
  }
  if (argv[3] <= 0){
    fprintf(stderr, "Invalid number of threads:\n");
    exit(EXIT_FAILURE);
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
  lock_function(1,lock_commands, rwlock_commands);
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
  if (fin==NULL){
    fprintf(stderr, "Error: Not existing input file\n");
    exit(EXIT_FAILURE);
  }
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
    unlock_function(lock_commands, rwlock_commands);
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
    int hashcode=hash(name,MAX);
    int searchResult;
    int iNumber;
    switch (token) {
      case 'c':
        iNumber = obtainNewInumber(fs);
        create(fs, name, iNumber, hashcode);
        break;
      case 'l':
        searchResult = lookup(fs, name, hashcode);
        if(!searchResult)
          printf("%s not found\n", name);
        else
          printf("%s found with inumber %d\n", name, searchResult);
        break;
      case 'd':
        delete(fs, name, hashcode);
        break;
      default: { /* error */
        fprintf(stderr, "Error: command to apply\n");
        exit(EXIT_FAILURE);
      }
    }
  }
  return NULL;
}

void apply_command_main(int maxThreads){
    #if defined(MUTEX) || defined(RWLOCK)
      for (int i=0;i<maxThreads;i++)
        if (!pthread_create(&tid[i],NULL,applyCommands,NULL))
          fprintf(stderr, "Error: pthread_create failed to execute\n");
      for (int i=0;i<maxThreads;i++)
        if (!pthread_join(tid[i],NULL))
          fprintf(stderr, "Error: pthread_join failed to execute\n");
    #else
      applyCommands(NULL);
    #endif
}

int main(int argc, char* argv[]) {
  parseArgs(argc, argv);
  
  FILE *fout;
  double time_taken=0;
  struct timeval start, end;

  lock_init();
  fs = new_tecnicofs(atoi(argv[4]));
  MAX=atoi(argv[4]);
  processInput(argv[1]);

  gettimeofday(&start, NULL); /*Start clock*/
  apply_command_main(atoi(argv[3]));
  gettimeofday(&end, NULL);   /*Ends clock*/
  fout = fopen(argv[2],"w");
  print_tecnicofs_tree(fout, fs);

  fclose(fout);
  lock_destroy();
  free_tecnicofs(fs);

  /*Execution Time*/
  time_taken = (end.tv_sec - start.tv_sec); /*Seconds*/
  time_taken += (end.tv_usec - start.tv_usec) * 1e-6; /*Micro-Seconds*/
  printf("TecnicoFS completed in %.04f seconds.\n", time_taken);
  exit(EXIT_SUCCESS);
}
