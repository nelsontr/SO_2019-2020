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
pthread_t vector_threads[MAX_CLIENTS];
uid_t clients[MAX_CLIENTS];
pthread_mutex_t lock;

int flag_acabou=0;
struct ucred ucred;
TIMER_T startTime, stopTime;

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
  //pthread_mutex_init(&lock,NULL);
}


int isPermitted(permission othersPermission, enum permission perm) {
  int ret;
  switch (othersPermission) {
		case 1:
		  if (perm == WRITE || perm == RW) ret = 0;
		  else ret = -1;
		  break;
		case 2:
		  if (perm == READ || perm == RW) ret = 0;
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
      if (isPermitted(othersPermission,perm) == 0) return 0;
    } 
    if (isPermitted(ownerPermission,files[fd].mode) == 0) return 0;
  }
  return TECNICOFS_ERROR_INVALID_MODE;
}

int apply_create(uid_t userid, char *buff){
  int iNumber=0;
  int permissions;
  char token,name[MAX_INPUT_SIZE];
  permission ownerPermissions,otherPermissions;

  sscanf(buff, "%s %s %d", &token, name, &permissions);
  if (lookup(fs,name)==-1){
    otherPermissions = permissions%10;
    ownerPermissions = permissions/10;
    iNumber = inode_create(userid,ownerPermissions,otherPermissions);
    
    create(fs, name, iNumber,1);
    return 0;
  }
  
  return TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
}

int apply_delete(uid_t userid, char *buff){
  int iNumber=0;
  uid_t owner;
  char token,name[MAX_INPUT_SIZE];

  sscanf(buff, "%s %s", &token, name);
  iNumber=lookup(fs,name);
  
  if (iNumber!=-1){
    inode_get(iNumber,&owner,NULL,NULL,NULL,0);
    if( userid==owner){
      inode_delete(iNumber);
      
      delete(fs, name,0);
      return 0;
    }
  }
  else if (iNumber==-1){
    
    return TECNICOFS_ERROR_FILE_NOT_FOUND;
  }
  
  return TECNICOFS_ERROR_PERMISSION_DENIED;
}

int apply_rename(uid_t userid, char *buff){
  int iNumberold=0, iNumbernew=0;
  uid_t ownerold;
  char token,nameold[MAX_INPUT_SIZE], namenew[MAX_INPUT_SIZE];
  permission ownerPermissions,otherPermissions;

  sscanf(buff, "%s %s %s", &token, nameold, namenew);

  iNumberold=lookup(fs,nameold);
  iNumbernew=lookup(fs,namenew);
  if (iNumberold==-1) {return TECNICOFS_ERROR_FILE_NOT_FOUND;}

  if (iNumbernew!=-1) {return TECNICOFS_ERROR_FILE_ALREADY_EXISTS;}

  inode_get(iNumberold,&ownerold,&ownerPermissions,&otherPermissions,NULL,0);
  
  
  if (ownerold==userid){
    renameFile(nameold, namenew, fs);
    return 0;
  } 
  else return TECNICOFS_ERROR_PERMISSION_DENIED;
}

int apply_open(uid_t userid, char* buff,struct file *files){
  int iNumber=0;
  uid_t owner;
  char token,name[MAX_INPUT_SIZE];
  enum permission mode, ownerperm,otherperm;

  sscanf(buff, "%s %s %u", &token, name, &mode);
  iNumber = lookup(fs,name);
  if (iNumber == -1) return TECNICOFS_ERROR_FILE_NOT_FOUND;
  inode_get(iNumber,&owner,&ownerperm,&otherperm,NULL,0);
  
  for(int i=0; i<5; i++)
    if (files[i].iNumber==iNumber) return TECNICOFS_ERROR_FILE_IS_OPEN;
  
  for(int i=0; i<5; i++)
    if (files[i].iNumber==-1){
      files[i].iNumber=iNumber;
      files[i].mode=mode;
      return i;
    }
  return TECNICOFS_ERROR_MAXED_OPEN_FILES;
}

int apply_close(uid_t userid, char* buff,struct file *files){
  int fileDescriptor=-1;
  char token;
  sscanf(buff, "%s %d",&token, &fileDescriptor);
  
  if (fileDescriptor>5) return -4;
  files[fileDescriptor].iNumber=-1;
  files[fileDescriptor].mode=0;
  return 0;
}

int apply_write(uid_t userid, char* buff,struct file *files){
  int len, fd=-1;
  char token,name[MAX_INPUT_SIZE];

  sscanf(buff, "%s %d %s", &token, &fd, name);
  
  if (fd>5 || fd<0){ 
    
    return TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
  }

  
  if (user_allowed(userid,fd,files,WRITE) == 0){
    if (files[fd].iNumber == -1) return TECNICOFS_ERROR_FILE_NOT_OPEN;
    len = inode_set(files[fd].iNumber, name, strlen(name)); //NAO E ASSIM
    return len;
  }
  return TECNICOFS_ERROR_PERMISSION_DENIED;
}

int apply_read(int socket, uid_t userid, char* buff,struct file *files){
  int len=0, fd=-1;

  char token;

  sscanf(buff, "%s %d %d", &token, &fd, &len);

  char content[len];
  memset(content, '\0', len);

  if (fd>5 || fd<0){
    dprintf(socket, "%s %d", "e", TECNICOFS_ERROR_FILE_ALREADY_EXISTS);
    return TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
  }
  
  if (files[fd].iNumber == -1) {
    dprintf(socket, "%s %d", "e", TECNICOFS_ERROR_FILE_NOT_OPEN);
    return TECNICOFS_ERROR_FILE_NOT_OPEN;
  }

  int rc=user_allowed(userid,fd,files,READ);
  if (!rc) {
    inode_get(files[fd].iNumber,NULL,NULL,NULL,content,len-1);
    dprintf(socket, "%s %ld", content, strlen(content));
    return len; 
  }
  else if (rc == TECNICOFS_ERROR_INVALID_MODE){
    dprintf(socket, "%s %d", "e", TECNICOFS_ERROR_INVALID_MODE);
    return TECNICOFS_ERROR_INVALID_MODE;
  }
  else{
    dprintf(socket, "%s %d", "e", TECNICOFS_ERROR_PERMISSION_DENIED);
    return TECNICOFS_ERROR_PERMISSION_DENIED;
  }
}

void* applyComands(void *args){
  int userid = *(int*) args;
  sigset_t set;
  struct ucred owner;
  socklen_t len = sizeof(struct ucred);
  getsockopt(userid, SOL_SOCKET, SO_PEERCRED, &owner, &len);

  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  pthread_sigmask(SIG_BLOCK, &set, NULL);

  char buff[MAX];
  struct file files[TABELA_SIZE];
  for (int i=0;i<TABELA_SIZE;i++){
    files[i].iNumber = -1;
    files[i].mode = 0;
  }

  while(1){
    bzero(buff, MAX_INPUT_SIZE); 
    read(userid, buff, sizeof(buff));
    int n=sizeof(buff);
    buff[n]=0;

    int rc=0;
    
    char token = buff[0];
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
  return NULL;
}

int add_client(){
  socklen_t len = sizeof(struct ucred);
  getsockopt(newsockfd, SOL_SOCKET, SO_PEERCRED, &ucred, &len);

  //CHECKING EQUAL CLIENTS
  for (int i=0;i<MAX_CLIENTS;i++)
    if (ucred.uid == clients[i]) return TECNICOFS_ERROR_OPEN_SESSION;
  for (int i=0;i<MAX_CLIENTS;i++)
    if (clients[i]==-1) clients[i]=ucred.uid;
  return 0;
}


void socket_create(){
  int i=0;
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
    if (!flag_acabou) {
      newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &len);
        if (newsockfd < 0) puts("server: accept error");

      if (add_client()) return;
      
      if (pthread_create(&vector_threads[i++], NULL, applyComands, &newsockfd) != 0){
        puts("Erro");
      }
    }
    else return;
  }

}


void acabou(){
  FILE*out = fopen(global_outputFile, "w");
  flag_acabou=-1;
  printf("\nTerminando servidor, esperando clientes se desligarem...\n");
  for(int i=0;i<MAX_CLIENTS;i++)
    pthread_join(vector_threads[i],NULL);

  TIMER_READ(stopTime);
  fprintf(out, "TecnicoFS completed in %.4f seconds.\n", TIMER_DIFF_SECONDS(startTime, stopTime));
  print_tecnicofs_tree(out,fs);

  fflush(out);
  fclose(out);
  
  inode_table_destroy();
	unlink(nomeSocket);
  free_tecnicofs(fs);
  exit(EXIT_SUCCESS);
}



int main(int argc, char* argv[]) {
  parseArgs(argc,argv);
  signal(SIGINT, acabou);
  TIMER_READ(startTime);

  for(int i=0;i<MAX_CLIENTS;i++){
    vector_threads[i]=0;
    clients[i]=-1;
  }

  fs = new_tecnicofs(numBuckets);
  inode_table_init();
  socket_create();
}
