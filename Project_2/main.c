/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */

/*
    O semáforo deve ser utilizado para evitar que
    o programa encerre ao encher o vetor de comandos, ou seja,
    deve executar o sem_wait uma vez que alguma funcao tente
    mexer no vetor de comandos e o sem_post sempre que a funcao de
    remover comandos for realizada
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "fs.h"
#include "constants.h"
#include "lib/timer.h"
#include "sync.h"

char* global_inputFile = NULL;
char* global_outputFile = NULL;
int numberThreads = 0;
int hashMax = 0;
pthread_mutex_t commandsLock;
tecnicofs* fs;
int flag_acabou=0;
sem_t sem_prod, sem_cons;

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

int insertCommand(char* data){
  sem_wait(&sem_prod);
  mutex_lock(&commandsLock);
  strcpy(inputCommands[(numberCommands++)%MAX_COMMANDS], data);
  mutex_unlock(&commandsLock);
  sem_post(&sem_cons);
  return 1;
}

char* removeCommand() {
  if (!flag_acabou && headQueue==numberCommands)
    return inputCommands[(headQueue++)%MAX_COMMANDS];
  return NULL;
}

void errorParse(int lineNumber){
    fprintf(stderr, "Error: line %d invalid\n", lineNumber);
    exit(EXIT_FAILURE);
}

void* processInput(void *args){
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
        if (numTokens < 1) {
            continue;
        }
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
        printf("%s\n",line);
    }
    flag_acabou=numberCommands;
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


void* applyCommands(void* args){
    while(1){
        //SECÇÂO QUE DA ERRO - COMEÇO
        mutex_lock(&commandsLock);
        printf("head:%d\n",headQueue);
        printf("comm:%d\n",numberCommands);

        if (numberCommands==0 || !flag_acabou) {
          mutex_unlock(&commandsLock);
          puts("AQUI");
          sem_wait(&sem_cons);
        }

        //SECÇÂO QUE DA ERRO - FIM
        if(numberCommands > 0){
            const char* command = removeCommand();
            sem_post(&sem_prod);        //Allows producer to automaticly put another element
            if (command==NULL){
                mutex_unlock(&commandsLock);
                return NULL;
            }
            //puts("PRODUz");
            char token;
            char name1[MAX_INPUT_SIZE],name2[MAX_INPUT_SIZE];
            sscanf(command, "%c %s %s", &token, name1, name2);
            int iNumber;
            switch (token) {
                case 'c':
                    iNumber = obtainNewInumber(fs);
                    mutex_unlock(&commandsLock);
                    create(fs, name1, iNumber);
                    break;
                case 'l':
                    mutex_unlock(&commandsLock);
                    int searchResult = lookup(fs, name1);
                    if(!searchResult)
                        printf("%s not found\n", name1);
                    else
                        printf("%s found with inumber %d\n", name1, searchResult);
                    break;
                case 'd':
                    mutex_unlock(&commandsLock);
                    delete(fs, name1);
                    break;
                case 'r':
                    mutex_unlock(&commandsLock);
                    renameFile(name1,name2,fs);
                    break;
                default: { /* error */
                    mutex_unlock(&commandsLock);
                    fprintf(stderr, "Error: commands to apply\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
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
    sem_init(&sem_prod,0,MAX_COMMANDS);
    sem_init(&sem_cons,0,0);
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

    mutex_destroy(&commandsLock);
    sem_destroy(&sem_cons);
    sem_destroy(&sem_prod);
    free_tecnicofs(fs);
    exit(EXIT_SUCCESS);
}
