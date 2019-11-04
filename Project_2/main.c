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
#include "lib/hash.h"

char* global_inputFile = NULL;
char* global_outputFile = NULL;
int numberThreads = 0;
pthread_mutex_t commandsLock;
tecnicofs* fs;
sem_t sem_prod, sem_cons;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;
int hashMax=0;

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
}

int insertCommand(char* data) {
    // Penso que nessa funcao quando o vetor de comandos estiver cheio devemos executar esses comandos e 
    // colocar o valor de numberCommands a 0
    if(numberCommands != MAX_COMMANDS) {
        strcpy(inputCommands[numberCommands++], data);
        sem_post(&sem_cons);
        puts("4\n");
        return 1;
    }
    return 0;
}

char* removeCommand() {
    if(numberCommands > 0){
        numberCommands--;
        return inputCommands[headQueue++];  
    }
    if (headQueue==MAX_COMMANDS){
        headQueue=0;
    }
    return NULL;
}


void* producer(void* data) {
    data = (char**)data;
    //if (!sem_wait(&sem)); //Se o valor do semáforo for menor ou igual a 0
        // Executar comandos existentes no vetor 
        // Esvaziar vetor para receber mais
        // Executar sem_post
    
    insertCommand(data);
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
        if (lineNumber==MAX_COMMANDS){
            sem_wait(&sem_prod);
            puts("Mete mais 1\n");
            lineNumber--;
        }
    }
    fclose(inputFile);
    sem_destroy(&sem_cons);
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
        sem_wait(&sem_cons);
        mutex_lock(&commandsLock);
        printf("»»%d\n",numberCommands);
        if(numberCommands > 0){
            const char* command = removeCommand();
            sem_post(&sem_prod);
            printf("Thread\n");
            if (command == NULL){
                mutex_unlock(&commandsLock);
                printf("OJK\n");
                return NULL;
            }
            char token;
            char name[MAX_INPUT_SIZE];
            sscanf(command, "%c %s", &token, name);
            printf("%s %s\n",&token,name);
            int hashcode=hash(name,hashMax);
            int iNumber;
            switch (token) {
                case 'c':
                    iNumber = obtainNewInumber(fs);
                    mutex_unlock(&commandsLock);
                    create(fs, name, iNumber, hashcode);
                    break;
                case 'l':
                    mutex_unlock(&commandsLock);
                    int searchResult = lookup(fs, name, hashcode);
                    if(!searchResult)
                        printf("%s not found\n", name);
                    else
                        printf("%s found with inumber %d\n", name, searchResult);
                    break;
                case 'd':
                    mutex_unlock(&commandsLock);
                    delete(fs, name, hashcode);
                    break;
                default: { /* error */
                    mutex_unlock(&commandsLock);
                    fprintf(stderr, "Error: commands to apply\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
        else{
            mutex_unlock(&commandsLock);
            return NULL;
        }
    }
}

void runThreads(FILE* timeFp){
    TIMER_T startTime, stopTime;
    pthread_t* workers = (pthread_t*) malloc((numberThreads) * sizeof(pthread_t));
    pthread_t producer_th;
    

    TIMER_READ(startTime);
    int err = pthread_create(&producer_th, NULL, processInput, NULL);
    if (err != 0){
        perror("Can't create thread");
        exit(EXIT_FAILURE);
    }
    
    printf("%d\n",numberThreads);

    for(int i = 0; i < numberThreads; i++){
        int err = pthread_create(&workers[i], NULL, applyCommands, NULL);
        if (err != 0){
            perror("Can't create thread");
            exit(EXIT_FAILURE);
        }
    }
    puts("Ok2\n");
    for(int i = 0; i < numberThreads; i++) {
        if(pthread_join(workers[i], NULL)) {
            perror("Can't join thread");
        }
    }
    puts("Ok3\n");
    if(pthread_join(producer_th, NULL)){
        perror("Can't join thread");
    }
    puts("Ok4\n");
    sem_destroy(&sem_cons);
    sem_destroy(&sem_prod);
    TIMER_READ(stopTime);
    fprintf(timeFp, "TecnicoFS completed in %.4f seconds.\n", TIMER_DIFF_SECONDS(startTime, stopTime));
    free(workers);
}

int main(int argc, char* argv[]) {
    parseArgs(argc, argv);
    FILE * outputFp = openOutputFile();
    mutex_init(&commandsLock);
    sem_init(&sem_prod,0,0);
    sem_init(&sem_cons,0,0);    
    hashMax=atoi(argv[4]);
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
