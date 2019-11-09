/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "fs.h"
#include "constants.h"
#include "lib/timer.h"
#include "sync.h"

/*GLOBAL VARIABLES*/
char* global_inputFile = NULL;
char* global_outputFile = NULL;
int hashMax = 0;
int numberThreads = 0;

pthread_mutex_t commandsLock;
tecnicofs* fs;
sem_t pode_prod, pode_cons;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

static void displayUsage (const char* appName){
    printf("Usage: %s input_filepath output_filepath threads_number\n", appName);
    exit(EXIT_FAILURE);
}

static void parseArgs (long argc, char* const argv[]){
    if (argc != 5) {
        fprintf(stderr, "Invalid format:\n");
        displayUsage(argv[0]);
    }

    global_inputFile = argv[1];
    global_outputFile = argv[2];
    numberThreads = atoi(argv[3]);
    if (!numberThreads) {
        fprintf(stderr, "Invalid number of threads\n");
        displayUsage(argv[0]);
    }
    hashMax=atoi(argv[4]);
    if(!hashMax){
        fprintf(stderr, "Invalid number of Hash Size\n");
        displayUsage(argv[0]);
    }
}

int insertCommand(char* data) {
    sem_wait_err(&pode_prod, "Producer");
    mutex_lock(&commandsLock);
    strcpy(inputCommands[(numberCommands++)%MAX_COMMANDS], data);
    mutex_unlock(&commandsLock);
    sem_post_err(&pode_cons, "Consumer");
    return 1;
}

char* removeCommand() {
    return inputCommands[(headQueue++)%MAX_COMMANDS];
}

void errorParse(int lineNumber){
    fprintf(stderr, "Error: line %d invalid\n", lineNumber);
    exit(EXIT_FAILURE);
}

void* processInput(void*agrs){
    FILE* inputFile;
    inputFile = fopen(global_inputFile, "r");
    if(!inputFile){
        fprintf(stderr, "Error: Could not read %s\n", global_inputFile);
        exit(EXIT_FAILURE);
    }
    char line[MAX_INPUT_SIZE];
    int lineNumber = 0;

    while (fgets(line, sizeof(line)/sizeof(char), inputFile)) {
        char token;
        char name[MAX_INPUT_SIZE];
        lineNumber++;
        int numTokens = sscanf(line, "%c %s", &token, name);

        /* perform minimal validation */
        if (numTokens < 1) { continue; }
        switch (token) {
            case 'c':
            case 'l':
            case 'd':
            case 'r':
                if(numTokens != 2)
                    errorParse(lineNumber);
                if(insertCommand(line))
                    break;
                return NULL;
            case '#':
                break;
            default: { /* error */
                errorParse(lineNumber);
            }
        }
    }
    for (int i=0;i<numberThreads;i++)
      insertCommand("e"); //Command e: end a Consumer thread
    fclose(inputFile);
    return NULL;
}

FILE * openOutputFile() {
    FILE *fp;
    fp = fopen(global_outputFile, "w");
    if (fp == NULL) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }
    return fp;
}

void* applyCommands(){
   while(1){
        sem_wait_err(&pode_cons, "Consumer");
        mutex_lock(&commandsLock);
        const char* command = removeCommand();
        char token;
        char name[MAX_INPUT_SIZE],name2[MAX_INPUT_SIZE];
        sscanf(command, "%c %s", &token, name);
        int iNumber;
        switch (token) {
            case 'c':
                iNumber = obtainNewInumber(fs);
                mutex_unlock(&commandsLock);
                sem_post_err(&pode_prod,"Producer");		        
                create(fs, name, iNumber,0);
                break;
            case 'l':
                mutex_unlock(&commandsLock);
                sem_post_err(&pode_prod,"Producer");		        
                int searchResult = lookup(fs, name);
                if(!searchResult)
                    printf("%s not found\n", name);
                else
                    printf("%s found with inumber %d\n", name, searchResult);
                break;
            case 'd':
                mutex_unlock(&commandsLock);
                sem_post_err(&pode_prod,"Producer");		        
                delete(fs, name,0);
                break;
            case 'r':
                sscanf(command, "%c %s %s", &token, name, name2);
                mutex_unlock(&commandsLock);
                sem_post_err(&pode_prod,"Producer");		        
                renameFile(name,name2,fs);
                break;
            case 'e':
                mutex_unlock(&commandsLock);
                //sem_post_err(&pode_prod,"Producer");		        
                return NULL;
                break;
            default: { /* error */
                mutex_unlock(&commandsLock);
                fprintf(stderr, "Error: commands to apply\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    return NULL;
}

void runThreads(FILE* timeFp){
    TIMER_T startTime, stopTime;
    pthread_t producer_th;
    pthread_t* workers = (pthread_t*) malloc((numberThreads) * sizeof(pthread_t));

    TIMER_READ(startTime);

    if (pthread_create(&producer_th, NULL, processInput, NULL)!= 0){
        perror("Can't create thread: Producer");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < numberThreads; i++){
        if (pthread_create(&workers[i], NULL, applyCommands, NULL)!= 0){
            perror("Can't create thread: Consumers");
            exit(EXIT_FAILURE);
        }
    }

    for(int i = 0; i < numberThreads; i++)
        if(pthread_join(workers[i], NULL))
            perror("Can't join thread: Consumers");

    if(pthread_join(producer_th, NULL))
        perror("Can't join thread: Producer");

    TIMER_READ(stopTime);
    fprintf(timeFp, "TecnicoFS completed in %.4f seconds.\n", TIMER_DIFF_SECONDS(startTime, stopTime));
    free(workers);
}

void init_variables(){
    mutex_init(&commandsLock);
    sem_init(&pode_prod,0,MAX_COMMANDS);
    sem_init(&pode_cons,0,0);
}

void destroy_variables(){
  mutex_destroy(&commandsLock);
  sem_destroy(&pode_cons);
  sem_destroy(&pode_prod);
}

int main(int argc, char* argv[]) {
    parseArgs(argc, argv);
    FILE * outputFp = openOutputFile();
    init_variables();
    fs = new_tecnicofs(hashMax);

    runThreads(stdout);
    print_tecnicofs_tree(outputFp, fs);
    fflush(outputFp);
    fclose(outputFp);

    destroy_variables();
    free_tecnicofs(fs);
    exit(EXIT_SUCCESS);
}
