/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */
/* Modified by Matheus and Nelson, group 22 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "fs.h"
#include "sync.h"
#include "constants.h"
#include "lib/timer.h"
#include "lib/inodes.h"

#define MAX 150

int sockfd, newsockfd;
tecnicofs *fs;
pthread_t *vector_threads;
pthread_mutex_t lock;
char *nomeSocket, *global_outputFile;
int numBuckets;


static void displayUsage (const char* appName){
    printf("Usage: %s input_filepath output_filepath threads_number\n", appName);
    exit(EXIT_FAILURE);
}

static void parseArgs (long argc, char* const argv[]){
    if (argc != 4) {
        fprintf(stderr, "Invalid format:\n");
        displayUsage(argv[0]);
    }
    
    nomeSocket = argv[1];
    global_outputFile = argv[2];
    numBuckets = atoi(argv[3]);
    if (!numBuckets) {
        fprintf(stderr, "Invalid number of buckets\n");
        displayUsage(argv[0]);
    }
}





void apply_create(int userid, char *buff){
  int iNumber=0;
  uid_t owner;
  char token,name[MAX_INPUT_SIZE];
  permission ownerPermissions,otherPermissions;
  sscanf(buff, "%s %s %d", &token, name, &owner);
  if (lookup(fs,name)==-1){
    otherPermissions = owner%10;
    ownerPermissions = owner/10;
    iNumber = inode_create(userid,ownerPermissions,otherPermissions);
    create(fs, name, iNumber,0);
    mutex_unlock(&lock);
    dprintf(userid,"%d",0);
  }
  else{
    mutex_unlock(&lock);
    dprintf(userid,"%d", -4);
  }
}

void apply_delete(int userid, char *buff){
  int iNumber=0;
  uid_t owner;
  char token,name[MAX_INPUT_SIZE];

  sscanf(buff, "%s %s", &token, name);
  iNumber=lookup(fs,name);
  if (iNumber==-1){
    dprintf(userid,"%d",TECNICOFS_ERROR_FILE_NOT_FOUND);
    return;
  }  
  inode_get(iNumber,&owner,NULL,NULL,NULL,0);
  if ((iNumber)!=-1){
    delete(fs, name,0);
    inode_delete(iNumber);
    dprintf(userid,"%d",0);
  } 
  else dprintf(userid,"%d",TECNICOFS_ERROR_PERMISSION_DENIED);
}

void apply_rename(){return;}

void apply_open(int userid, char* buff,int *files){
  int iNumber=0;
  uid_t owner;
  char token,name[MAX_INPUT_SIZE], mode[MAX_INPUT_SIZE];

  sscanf(buff, "%s %s %s", &token, name, mode);
  if ((iNumber = lookup(fs,name))!=-1){
    inode_get(iNumber,&owner,NULL,NULL,NULL,0);
    
    if (userid != owner) {
      dprintf(userid,"%d",-6);
      return;
    }
    
    for(int i=0; i<5; i++)
      if (files[i]==-1){
        files[i]=iNumber;
        dprintf(userid,"%d",i);
        return;
      }
  }
  else dprintf(userid, "%d", -4);
}

void apply_close(int userid, char* buff,int *files){
  int fileDescriptor=-1;
  char token;
  sscanf(buff, "%s %d",&token, &fileDescriptor);
  if (fileDescriptor>5){ 
    dprintf(userid, "%d", -4);
    return;
  }
  files[fileDescriptor]=-1;
  dprintf(userid, "%d", 0);
}

void apply_write(){return;}

void apply_read(int userid, char* buff,int *files){
  int len=0, fd=-1;
  uid_t owner;
  char token,name[MAX_INPUT_SIZE], content[MAX_INPUT_SIZE];

  sscanf(buff, "%s %d %s %d", &token, &fd, name, &len);

  if (fd>5){ 
    dprintf(userid, "%d", -4);
    return;
  }

  inode_get(files[fd],&owner,NULL,NULL,content,len);
  strcpy(name, content);
  dprintf(userid, "%d", len);
}



void* applyComands(void *args){
  int userid = *(int*) args;
  char buff[MAX];
	bzero(buff, MAX); 
  int n = sizeof(buff);

  int *files = malloc(sizeof(int)*5);
  for (int i=0;i<5;i++)
    files[i]=-1;
  inode_table_init();

  while(read(userid, buff, n)){
    mutex_lock(&lock);
    buff[n]=0;
    
    char token = buff[0];
    switch (token) {
      case 'c':
        apply_create(userid, buff);
        break;

      case 'd':
        apply_delete(userid, buff);
        break;

      case 'r':
        apply_rename(userid,buff);

      case 'o':
        apply_open(userid, buff,files);
        break;

      case 'x':
        apply_close(userid, buff, files);
        break;

      case 'l':
        apply_read(userid, buff, files);
        break;
      case 'w':
        apply_write(userid,buff,files);
        break;

      case 'e':
        return NULL;
      default:
        //ERRO
        return NULL;
    }
  print_tecnicofs_tree(stdout,fs);
  }
  return NULL;
}

void socket_create(){
  struct sockaddr_un serv_addr, cli_addr;
  
	sockfd = socket(AF_UNIX,SOCK_STREAM,0);
  if (sockfd < 0)
    puts("server: can't open stream socket");
    
	unlink(nomeSocket);
  bzero((char *)&serv_addr, sizeof(serv_addr));

  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, nomeSocket);
  int servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
  
  if (bind(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
    puts("server, can't bind local address");

  listen(sockfd, 5);
  
      
  for (;;){
    socklen_t len = sizeof(cli_addr);
    int i=0;
    newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &len);
    if (newsockfd < 0) puts("server: accept error");
    
    if (pthread_create(&vector_threads[i++], NULL, applyComands, &newsockfd) != 0){
      puts("Erro");
    }
  }
}


int main(int argc, char* argv[]) {
  parseArgs(argc,argv);

  vector_threads = malloc(sizeof(pthread_t)*10);
  fs = new_tecnicofs();
  socket_create();
  exit(EXIT_SUCCESS);
}