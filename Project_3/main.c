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
#define TABELA_SIZE 5

int numBuckets;
char *nomeSocket, *global_outputFile;
int sockfd, newsockfd;

tecnicofs *fs;
pthread_t *vector_threads;
int *clients;
pthread_mutex_t lock;

int flag_acabou=0;
struct ucred ucred;


struct file {
  int iNumber;
  enum permission mode;
};


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


int isPermitted(permission othersPermission, enum permission perm) {
  int ret;
  switch (othersPermission) {
  
  case 1:
    if (perm == WRITE || perm == RW)
      ret = 0;
    else ret = -1;
    break;
  case 2:
    if (perm == READ || perm == RW)
      ret = 0;
    else ret = -1;
    break;
  case 3:
    ret = 0;
    break;
  case 0:
  default:
    ret = -1;
    break;
  }
  return ret;
}


int user_allowed(int userid, int fd, struct file *files, enum permission perm) {
  permission ownerPermission;
  permission othersPermission;
  uid_t creatorId;
  
  inode_get(files[fd].iNumber,&creatorId,&ownerPermission,&othersPermission,NULL,0);
  if (isPermitted(perm,files[fd].mode) == 0) {
    if (userid != creatorId) {
      if (isPermitted(othersPermission,perm) == 0) {
        return 0;
      }
    } else if (isPermitted(ownerPermission,files[fd].mode) == 0) {
      return 0;
    }
  }
  return 1;
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
    return TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
  }
}

int apply_delete(int userid, char *buff){
  int iNumber=0;
  uid_t owner;
  char token,name[MAX_INPUT_SIZE];

  sscanf(buff, "%s %s", &token, name);
  iNumber=lookup(fs,name);
  if (iNumber==-1){
    mutex_unlock(&lock);
    return TECNICOFS_ERROR_FILE_NOT_FOUND;
  } 
  inode_get(iNumber,&owner,NULL,NULL,NULL,0);
  if (iNumber!=-1 && userid==owner){
    inode_delete(iNumber);
    mutex_unlock(&lock);
    delete(fs, name,0);
    return 0;
  } 
  else {
    mutex_unlock(&lock);
    return TECNICOFS_ERROR_PERMISSION_DENIED;
  }
}

int apply_rename(int userid, char *buff){
  int iNumberold=0, iNumbernew=0;
  uid_t ownerold;//,ownernew;
  char token,nameold[MAX_INPUT_SIZE], namenew[MAX_INPUT_SIZE];
  permission ownerPermissions,otherPermissions;

  sscanf(buff, "%s %s %s", &token, nameold, namenew);

  iNumberold=lookup(fs,nameold);
  if (iNumberold==-1){
    return TECNICOFS_ERROR_FILE_NOT_FOUND;
  }
  iNumbernew=lookup(fs,namenew);
  if (iNumbernew!=-1){
    return TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
  }

  inode_get(iNumberold,&ownerold,&ownerPermissions,&otherPermissions,NULL,0);
  
  if (ownerold==userid){
    renameFile(nameold, namenew, fs);
    return 0;
  } 
  else return TECNICOFS_ERROR_PERMISSION_DENIED;
}

int apply_open(int userid, char* buff,struct file *files){
  int iNumber=0;
  uid_t owner;
  char token,name[MAX_INPUT_SIZE];
  enum permission mode;

  sscanf(buff, "%s %s %u", &token, name, &mode);
  if ((iNumber = lookup(fs,name))!=-1){
    inode_get(iNumber,&owner,NULL,NULL,NULL,0);
    
    if (userid != owner) {
      mutex_unlock(&lock);
      return -6;
    }

    for(int i=0; i<5; i++)
      if (files[i].iNumber==iNumber)
      return -4;
    
    for(int i=0; i<5; i++)
      if (files[i].iNumber==-1){
        files[i].iNumber=iNumber;
        files[i].mode=mode;
        mutex_unlock(&lock);
        if (user_allowed(userid,i,files,mode) == 0)
          return i;
        else 
          return TECNICOFS_ERROR_PERMISSION_DENIED;
      }
  }
  mutex_unlock(&lock);
  return -4;
}

int apply_close(int userid, char* buff,struct file *files){
  int fileDescriptor=-1;
  char token;
  sscanf(buff, "%s %d",&token, &fileDescriptor);
  if (fileDescriptor>5) return -4;
  files[fileDescriptor].iNumber=-1;
  mutex_unlock(&lock);
  return 0;
}

int apply_write(int userid, char* buff,struct file *files){
  int len, fd=-1;
  uid_t owner;
  char token,name[MAX_INPUT_SIZE], *content=NULL;

  sscanf(buff, "%s %d %s", &token, &fd, name);
  
  if (fd>5 || fd<0){ 
  mutex_unlock(&lock);
    return -4;
  }
  mutex_unlock(&lock);
  if (user_allowed(userid,fd,files,WRITE) == 0){
    len = inode_set(files[fd].iNumber, name, strlen(name)); //NAO E ASSIM
    return len;
  } else {
    return TECNICOFS_ERROR_PERMISSION_DENIED;
  }
}

int apply_read(int socket, int userid, char* buff,struct file *files){
  int len=0, fd=-1;
  uid_t owner;


  char token;

  sscanf(buff, "%s %d %d", &token, &fd, &len);

  char content[len];
  memset(content, '\0', sizeof(content));

  if (fd>5 || fd<0){ 
    mutex_unlock(&lock);
    return -4;
  }

  mutex_unlock(&lock);
  if (user_allowed(userid,fd,files,READ) == 0) {
    inode_get(files[fd].iNumber,NULL,NULL,NULL,content,len);
    dprintf(socket, "%s", content);
    return len; 
  } else {
    return TECNICOFS_ERROR_PERMISSION_DENIED;
  }
}

void* applyComands(void *args){
  int userid = *(int*) args;
  struct ucred owner;
  socklen_t len = sizeof(struct ucred);
  getsockopt(userid, SOL_SOCKET, SO_PEERCRED, &owner, &len);

  struct file *files = malloc(sizeof(struct file)*TABELA_SIZE);
  for (int i=0;i<TABELA_SIZE;i++){
    files[i].iNumber = -1;
    files[i].mode = 0;
  }
  char buff[MAX];

  while(1){
    mutex_lock(&lock);
    bzero(buff, MAX_INPUT_SIZE); 
    read(userid, buff, sizeof(buff));
    int n=sizeof(buff);
    buff[n]=0;

    int rc=0;
    
    char token = buff[0];
    printf("%c\n",token);
    switch (token) {
      case 'c':
        rc = apply_create(owner.uid, buff);
        dprintf(userid,"%d",rc);
        break;
      case 'd':
        rc = apply_delete(owner.uid, buff);
        dprintf(userid,"%d",rc);
        break;
      case 'r':
        rc = apply_rename(owner.uid, buff);
        dprintf(userid,"%d",rc);
        break;
      case 'o':
        rc = apply_open(owner.uid, buff,files);
        dprintf(userid,"%d",rc);
        break;
      case 'x':
        rc = apply_close(owner.uid, buff, files);
        dprintf(userid,"%d",rc);
        break;
      case 'l':
        rc = apply_read(userid, owner.uid, buff, files);
        break;
      case 'w':
        rc = apply_write(owner.uid, buff, files);
        dprintf(userid,"%d",rc);
        break;
      case 'e':
        return NULL;
    }
  } 
  free(files);
  return NULL;
}

void socket_create(){
  int i=0;
  sigset_t set;
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

      /*sigemptyset(&set);
      sigaddset(&set, SIGINT);
      pthread_sigmask(SIG_SETMASK, &set, NULL);*/
      
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
  signal(SIGTERM, acabou);

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
