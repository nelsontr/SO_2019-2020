/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */
/* Modified by Matheus and Nelson, group 22 */
#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

#include "fs.h"
#include "sync.h"
#include "constants.h"
#include "lib/timer.h"
#include "lib/inodes.h"

#define MAX 150
#define MAX_CLIENTS 10

int numBuckets;
char *nomeSocket, *global_outputFile;
int sockfd, newsockfd;

tecnicofs *fs;
pthread_t *vector_threads;
int *clients;
pthread_mutex_t lock;

int flag_acabou=0;
struct ucred ucred;


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
  pthread_mutex_init(&lock,NULL);
}

int apply_create(int userid, char *buff){
  int iNumber=0;
  int permissions;
  char token,name[MAX_INPUT_SIZE];
  permission ownerPermissions,otherPermissions;
  sscanf(buff, "%s %s %d", &token, name, &permissions);
  if (lookup(fs,name)==-1){
    otherPermissions = permissions%10;
    ownerPermissions = permissions/10;
    iNumber = inode_create(userid,ownerPermissions,otherPermissions);
    mutex_unlock(&lock);
    create(fs, name, iNumber,1);
    return 0;
  }
  else{
    mutex_unlock(&lock);
    return TECNICOFS_ERROR_FILE_NOT_FOUND;
  }
}

void apply_delete(int userid, char *buff){
  int iNumber=0;
  uid_t owner;
  char token,name[MAX_INPUT_SIZE];

  sscanf(buff, "%s %s", &token, name);
  iNumber=lookup(fs,name);
  if (iNumber==-1){
    mutex_unlock(&lock);
    dprintf(userid,"%d",TECNICOFS_ERROR_FILE_NOT_FOUND);
    return;
  }  
  inode_get(iNumber,&owner,NULL,NULL,NULL,0);
  if (iNumber!=-1 && userid==owner){
    inode_delete(iNumber);
    mutex_unlock(&lock);
    delete(fs, name,0);
    dprintf(userid,"%d",0);
  } 
  else {
    mutex_unlock(&lock);
    dprintf(userid,"%d",TECNICOFS_ERROR_PERMISSION_DENIED);
  }
}

void apply_rename(int userid, char *buff){
  int iNumberold=0, iNumbernew=0;
  uid_t ownerold;//,ownernew;
  char token,nameold[MAX_INPUT_SIZE], namenew[MAX_INPUT_SIZE];
  permission ownerPermissions,otherPermissions;

  sscanf(buff, "%s %s %s", &token, nameold, namenew);

  iNumberold=lookup(fs,nameold);
  if (iNumberold==-1){
    dprintf(userid,"%d",TECNICOFS_ERROR_FILE_NOT_FOUND);
    return;
  }
  iNumbernew=lookup(fs,namenew);
  if (iNumbernew!=-1){
    dprintf(userid,"%d",TECNICOFS_ERROR_FILE_ALREADY_EXISTS);
    return;
  }

  inode_get(iNumberold,&ownerold,&ownerPermissions,&otherPermissions,NULL,0);
  
  if (ownerold==userid){

    renameFile(nameold, namenew, fs);
    //inode_delete(iNumberold);
    //inode_create(userid,ownerPermissions,otherPermissions);
    dprintf(userid,"%d",0);
  } 
  else dprintf(userid,"%d",TECNICOFS_ERROR_PERMISSION_DENIED);
  return;
}

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

  if (fd>5 || fd<0){ 
    dprintf(userid, "%d", -4);
    return;
  }

  inode_get(files[fd],&owner,NULL,NULL,content,len);
  strcpy(name, content);
  dprintf(userid, "%d", len);
}



void* applyComands(void *args){
  int userid = *(int*) args;
  struct ucred owner;
  socklen_t len = sizeof(struct ucred);
  getsockopt(userid, SOL_SOCKET, SO_PEERCRED, &owner, &len);

  char buff[MAX];
	bzero(buff, MAX); 
  int n = sizeof(buff);

  int *files = malloc(sizeof(int)*MAX_CLIENTS);
  for (int i=0;i<MAX_CLIENTS;i++)
    files[i]=-1;

  while(read(userid, buff, n)){
    int rc=0;
    mutex_lock(&lock);
    buff[n]=0;
    
    char token = buff[0];
    switch (token) {
      case 'c':
        rc = apply_create(owner.uid, buff);
        dprintf(userid,"%d",rc);
        break;
      case 'd':
        apply_delete(owner.uid, buff);
        break;
      case 'r':
        apply_rename(owner.uid, buff);
      case 'o':
        apply_open(owner.uid, buff,files);
        break;
      case 'x':
        apply_close(owner.uid, buff, files);
        break;
      case 'l':
        apply_read(owner.uid, buff, files);
        break;
      case 'w':
        apply_write(owner.uid, buff, files);
        break;
      case 'e':
        return NULL;
      default:
        fprintf(stderr, "Error: commands to apply\n");
        exit(EXIT_FAILURE);
    }
  }
  free(files);
  return NULL;
}

void socket_create(){
  int i=0;
  //struct ucred ucred;
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
  
  for (int i=0; i<MAX_CLIENTS;i++)
    clients[i]=-1;

  for (;;){
    socklen_t len = sizeof(cli_addr);
    if (!flag_acabou) {
      newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &len);
        if (newsockfd < 0) puts("server: accept error");
      if (pthread_create(&vector_threads[i++], NULL, applyComands, &newsockfd) != 0){
        puts("Erro");
      }
    }
    else return;
  /*
    len = sizeof(struct ucred);
    getsockopt(newsockfd, SOL_SOCKET, SO_PEERCRED, &ucred, &len);

    //CHECKING EQUAL CLIENTS
    for (int i=0;i<MAX_CLIENTS;i++)
      if (ucred.uid == clients[i]) return;
    for (int i=0;i<MAX_CLIENTS;i++)
      if (clients[i]==-1) clients[i]=ucred.uid;
    */
  }
}

int isPermitted(permission othersPermission, char *mode) {
  int ret;
  switch (othersPermission) {
  case 0:
    ret = -1;
    break;
  case 1:
    if (strcmp(mode, (char*)'W') || strcmp(mode,"RW") == 0)
      ret = 0;
    break;
  case 2:
    if (strcmp(mode, (char*)'R') || strcmp(mode,"RW") == 0)
      ret = 0;
    break;
  case 3:
    ret = 0;
    break;
  default:
    ret = -1;
    break;
  }
  return ret;
}


int user_allowed(int userid, int fd, char *mode, char *openMode){
  //Assumindo que ficheiro esta abero na Tabela
  //return 1 se ficheiro esta na tabela com um modo que nao é o indicado quando da create
  //return 0 se o utilizador pode fazer o que quer, 
  /*
    User cria a com ownerpermission: R
    A mesma coisa para outros

    Vamos escrever nesse ficheiros, o write chama esta função e tem de dar erro uma vez que 
    na tabela ele esta aberto como R nao como RW nem W.
    
  */
  permission ownerPermission;
  permission othersPermission;
  uid_t creatorId;
  
  inode_get(fd,&creatorId,&ownerPermission,&othersPermission,NULL,0);
  if (strcmp(mode, openMode) == 0) {
    if (userid != creatorId) {
      if (isPermitted(othersPermission,mode) == 0) {
        return 0;
      }
    } else if (isPermitted(othersPermission,openMode) == 0) {
      return 0;
    }
  }
  return 1;
}


void acabou(){
  FILE*out = fopen(global_outputFile, "w");
  flag_acabou=-1;
  printf("\nTerminando servidor, esperando clientes se desligarem...\n");
  for(int i=0;i<MAX_CLIENTS;i++)
    pthread_join(vector_threads[i],NULL);

  print_tecnicofs_tree(out,fs);
  fclose(out);
  inode_table_destroy();
  free_tecnicofs(fs);
  free(vector_threads);
  free(clients);
  exit(EXIT_SUCCESS);
}



int main(int argc, char* argv[]) {
  parseArgs(argc,argv);
  signal(SIGINT, acabou);

  vector_threads = malloc(sizeof(pthread_t)*MAX_CLIENTS);
  clients = malloc(sizeof(int)*MAX_CLIENTS);
  for(int i=0;i<MAX_CLIENTS;i++){
    vector_threads[i]=0;
    clients[i]=0;
  }
  inode_table_init();

  fs = new_tecnicofs();
  socket_create();
}