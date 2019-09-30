#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "fs.h"

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

int numberThreads = 0;
tecnicofs* fs;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

static void displayUsage (const char* appName){
    printf("Usage: %s\n", appName);
    exit(EXIT_FAILURE);
}

static void parseArgs (long argc, char* const argv[]){
    if (argc != 3) {
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
    if((numberCommands + 1)){
        numberCommands--;
        return inputCommands[headQueue++];
    }
    return NULL;
}

void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    //exit(EXIT_FAILURE);
}

void processInput(char* filename){
    char line[MAX_INPUT_SIZE];

    FILE *fp=fopen(filename,"r");
/*    while (fgets(, MAX_COMMANDS, fp )!=NULL){
      printf("%s",inputCommands[numberCommands-1]);
    }*/

    while (fgets(line, sizeof(line)/sizeof(char), fp)) {
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
                if(insertCommand(line)){
                    //printf("%s",inputCommands[numberCommands-1]);
                    break;}
                return;
            case '#':
                break;
            default: { /* error */
                errorParse();
            }
        }
    }
    fclose(fp);
}

void applyCommands(FILE* fout){
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
                    fprintf(fout,"%s not found\n", name);
                else
                    fprintf(fout,"%s found with inumber %d\n", name, searchResult);
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
}

int main(int argc, char* argv[]) {
  double time_spent;
  clock_t begin, end;
  FILE *fout=fopen(argv[2],"w");
  parseArgs(argc, argv);

  begin = clock();
  fs = new_tecnicofs();
  processInput(argv[1]);
  applyCommands(fout);
  print_tecnicofs_tree(fout, fs);

  free_tecnicofs(fs);
  end = clock();
  time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  fprintf(fout,"TecnicoFS completed in %.04f seconds.\n", time_spent);
  fclose(fout);
  exit(EXIT_SUCCESS);
}
