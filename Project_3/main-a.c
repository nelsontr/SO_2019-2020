/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */
/* Modified by Matheus and Nelson, group 22 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/un.h>

#include "fs.h"
#include "constants.h"
#include "lib/timer.h"
#include "sync.h"


#define UNIXSTR_PATH "/tmp/s.unixstr"
#define UNIXDG_PATH "/tmp/s.unixdgx"
#define UNIXDG_TMP "/tmp/dgXXXXXXX"


/*GLOBAL VARIABLES*/
char* nomesocket = NULL;
int numbuckets = 0;
outFile = NULL;

tecnicofs *fs;
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
    if (argc != 4) {
        fprintf(stderr, "Invalid format:\n");
        displayUsage(argv[0]);
    }

    nomesocket = argv[1];
    if (nomesocket==NULL){
        fprintf(stderr, "Invalid Name");
        displayUsage(argv[0]);
    }
    
    outFile = argv[2];
    if (outFile==NULL){
        fprintf(stderr, "Invalid Name");
        displayUsage(argv[0]);
    }
		
    numbuckets = atoi(argv[3]);
    if (!numbuckets){
        fprintf(stderr, "Invalid number of threads\n");
        displayUsage(argv[0]);
    }
}

int insertCommand(char* data) {
    sem_wait_err(&sem_prod, "Producer");
    mutex_lock(&commandsLock);
    strcpy(inputCommands[(numberCommands++)%MAX_COMMANDS], data);
    mutex_unlock(&commandsLock);
    sem_post_err(&sem_cons, "Consumer");
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
    int lineNumber = 0;
    char line[MAX_INPUT_SIZE];

    while (fgets(line, sizeof(line)/sizeof(char), inputFile)) {
        char token;
        char name[MAX_INPUT_SIZE];
        char name2[MAX_INPUT_SIZE];
        lineNumber++;
        int numTokens = sscanf(line, "%c %s %s", &token, name, name2);

        /* perform minimal validation */
        if (numTokens < 1) { continue; }
        switch (token){
            case 'r':
                if (numTokens != 3) errorParse(lineNumber);
                if(insertCommand(line))
                    break;
            case 'c':
            case 'l':
            case 'd':
                if(numTokens != 2) errorParse(lineNumber);
                if(insertCommand(line))
                    break;
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
    fp = fopen(outFile, "w");
    if (fp == NULL) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }
    return fp;
}

void* applyCommands(){
   while(1){
        sem_wait_err(&sem_cons, "Consumer");
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
                sem_post_err(&sem_prod,"Producer");		        
                create(fs, name, iNumber,0);
                break;
            case 'l':
                mutex_unlock(&commandsLock);
                sem_post_err(&sem_prod,"Producer");		        
                int searchResult = lookup(fs, name);
                if(!searchResult)
                    printf("%s not found\n", name);
                else
                    printf("%s found with inumber %d\n", name, searchResult);
                break;
            case 'd':
                mutex_unlock(&commandsLock);
                sem_post_err(&sem_prod,"Producer");		        
                delete(fs, name,0);
                break;
            case 'r':
                sscanf(command, "%c %s %s", &token, name, name2);
                mutex_unlock(&commandsLock);
                sem_post_err(&sem_prod,"Producer");		        
                renameFile(name,name2,fs);
                break;
            case 'e':
                mutex_unlock(&commandsLock);
                //sem_post_err(&sem_prod,"Producer");		        
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

/*void init_variables(){
    mutex_init(&commandsLock);
    sem_init(&sem_prod,0,MAX_COMMANDS);
    sem_init(&sem_cons,0,0);
}

void destroy_variables(){
  mutex_destroy(&commandsLock);
  sem_destroy(&sem_cons);
  sem_destroy(&sem_prod);
}*/


void socket_create(){
	struct sockaddr_un serv_addr, cli_addr;
	int sockfd;
	FILE* fout=fopen(argv[2],"w");
	sockfd = socket(AF_UNIX,SOCK_STREAM,0);
  if (sockfd < 0)
    puts("server: can't open stream socket");
    
	unlink(argv[1]);
  bzero((char *)&serv_addr, sizeof(serv_addr));

  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, argv[1]);
  int servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
  
  if (bind(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
    puts("server, can't bind local address");
  listen(sockfd, 5);
  int len=sizeof(cli_addr);
	
	int newsockfd = accept(sockfd,(struct sockaddr *) &serv_addr, &len);
	if (newsockfd < 0) puts("server: accept error");

}


void accepta(){
	for (;;) { 
		bzero(buff, MAX); 

		// read the message from client and copy it in buffer 
		read(sockfd, buff, sizeof(buff)); 
		// print buffer which contains the client contents 
		printf("From client: %s\t To client : ", buff); 
		bzero(buff, MAX); 
		n = 0; 
		// copy server message in the buffer 
		while ((buff[n++] = getchar()) != '\n') 
			; 

		// and send that buffer to client 
		write(sockfd, buff, sizeof(buff)); 

		// if msg contains "Exit" then server exit and chat ended. 
		if (strncmp("exit", buff, 4) == 0) { 
			printf("Server Exit...\n"); 
			break; 
		} 
	} 
}

int main(int argc, char* argv[]) {
    parseArgs(argc, argv);
    
    
    FILE * outputFp = openOutputFile();
    
    fs = new_tecnicofs(hashMax);

    socket_create();
		accepta();
    

    /*runThreads(stdout);*/
    print_tecnicofs_tree(outputFp, fs);
    fflush(outputFp);
    fclose(outputFp);

    destroy_variables();
    free_tecnicofs(fs);
    exit(EXIT_SUCCESS);
}
