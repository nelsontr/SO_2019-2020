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

int insertCommand(char* data) {
    // Penso que nessa funcao quando o vetor de comandos estiver cheio devemos executar esses comandos e
    // colocar o valor de numberCommands a 0
    sem_wait(&sem_prod);
    mutex_lock(&commandsLock);
    if(numberCommands != MAX_COMMANDS) {
        strcpy(inputCommands[(numberCommands++)%MAX_COMMANDS], data);
        mutex_unlock(&commandsLock);
        //puts("ENVIA");
        sem_post(&sem_cons);
        //puts("ENVIADO");
        return 1;
    }
    return 0;
}

char* removeCommand() {
    if ((headQueue+1%MAX_COMMANDS)==0){
        headQueue++;
        return inputCommands[0];
    }
    else if((headQueue-1)!=numberCommands){
        return inputCommands[(headQueue++)%MAX_COMMANDS];
    }
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
    //puts("Criada!");
    while(1){
        if (headQueue!=numberCommands || numberCommands==0){
            //puts("Aguardando...");
            sem_wait(&sem_cons);
            //puts("Entrei");
        }
        mutex_lock(&commandsLock);
        if(numberCommands > 0){
            if (headQueue==numberCommands){
                mutex_unlock(&commandsLock);
                return NULL;
            }
            const char* command = removeCommand();
            sem_post(&sem_prod);        //Allows producer to automaticly put another element
            //puts("PRODUz");
            char token;
            char name1[MAX_INPUT_SIZE],name2[MAX_INPUT_SIZE];
            sscanf(command, "%c %s %s", &token, name1, name2);
            int hashcode=hash(name1,hashMax);
            int iNumber;
            switch (token) {
                case 'c':
                    iNumber = obtainNewInumber(fs);
                    mutex_unlock(&commandsLock);
                    create(fs, name1, iNumber, hashcode);
                    break;
                case 'l':
                    mutex_unlock(&commandsLock);
                    int searchResult = lookup(fs, name1, hashcode);
                    if(!searchResult)
                        printf("%s not found\n", name1);
                    else
                        printf("%s found with inumber %d\n", name1, searchResult);
                    break;
                case 'd':
                    mutex_unlock(&commandsLock);
                    delete(fs, name1, hashcode);
                    break;
                case 'r':
                    mutex_unlock(&commandsLock);
                    renameFile(name1,name2,hashcode,fs,hashMax);
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
    pthread_t* workers = (pthread_t*) malloc((numberThreads) * sizeof(pthread_t));
    pthread_t producer_th;

    TIMER_READ(startTime);
    if (pthread_create(&producer_th, NULL, processInput, NULL)!= 0){
        perror("Can't create thread");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < numberThreads; i++){
        if (pthread_create(&workers[i], NULL, applyCommands, NULL)!= 0){
            perror("Can't create thread");
            exit(EXIT_FAILURE);
        }
    }

    for(int i = 0; i < numberThreads; i++) {
        if(pthread_join(workers[i], NULL)) {
            perror("Can't join thread");
        }
    }

    if(pthread_join(producer_th, NULL)){
        perror("Can't join thread");
    }

    TIMER_READ(stopTime);
    fprintf(timeFp, "TecnicoFS completed in %.4f seconds.\n", TIMER_DIFF_SECONDS(startTime, stopTime));
    sem_destroy(&sem_cons);
    sem_destroy(&sem_prod);
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
    free_tecnicofs(fs);
    exit(EXIT_SUCCESS);
}
