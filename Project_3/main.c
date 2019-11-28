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
pthread_t *vector_threads;
tecnicofs *fs;

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
  buff[n]=0;
  permission ownerPermissions,otherPermissions;
  int iNumber=0;
  int fd,len;
  char token,name[MAX_INPUT_SIZE], mode[MAX_INPUT_SIZE];
  char content[MAX_INPUT_SIZE];
  uid_t owner;
  int fileDescriptor;
  sscanf(buff, "%s", &token);
  switch (token) {
      case 'c':
        sscanf(buff, "%s %s %d", &token, name, &owner);
        if (lookup(fs,name)==-1){
          otherPermissions = owner%10;
          ownerPermissions = owner/10;

          iNumber = inode_create(userid,ownerPermissions,otherPermissions);
          create(fs, name, iNumber,0);
          dprintf(userid,"%d",0);
        }
        else dprintf(userid,"%d", -4);
        break;

      case 'd':
        sscanf(buff, "%s %s", &token, name);
        iNumber=lookup(fs,name);
        if (iNumber==-1){
          dprintf(userid,"%d",TECNICOFS_ERROR_FILE_NOT_FOUND);
          break;
        }  
        inode_get(iNumber,&owner,NULL,NULL,NULL,0);
        if ((iNumber)!=-1 && userid == owner){
          delete(fs, name,0);
          inode_delete(iNumber);
          dprintf(userid,"%d",0);
        } 
        else dprintf(userid,"%d",TECNICOFS_ERROR_PERMISSION_DENIED);
        break;

      case 'o':
        sscanf(buff, "%s %s %s", &token, name, mode);
        if ((iNumber = lookup(fs,name))!=-1){
          inode_get(iNumber,&owner,NULL,NULL,NULL,0);
          
          if (userid != owner) {
            dprintf(userid,"%d",-6);
            break;
          }
          
          for(int i=0; i<5; i++)
            if (files[i]==-1){
              files[i]=iNumber;
              dprintf(userid,"%d",i);
              break;
            }
          dprintf(userid,"%d",TECNICOFS_ERROR_MAXED_OPEN_FILES);
        }
        else dprintf(userid, "%d", -4);
        break;

      case 'x':
        sscanf(buff, "%d", &fileDescriptor);
        if (fileDescriptor>5){ 
          dprintf(userid, "%d", -4);
          break;
        }
        if (files[fileDescriptor]!=-1)
          files[fileDescriptor]=-1;
        else dprintf(userid, "%d", TECNICOFS_ERROR_FILE_NOT_OPEN);
        dprintf(userid, "%d", 0);
        break;

      case 'l':

        sscanf(buff, "%s %d %s %d", &token, &fd, name, &len);
        if (fileDescriptor>5){ 
          dprintf(userid, "%d", -4);
          break;
        }
        inode_get(files[fileDescriptor],&owner,NULL,NULL,content,len);
        strcpy(name, content);
        dprintf(userid, "%d", len);
        break;

      case 'r':
        break;
      case 'e':
        return NULL;
  
  //printf("%s\n", buff); 
  }
  print_tecnicofs_tree(stdout,fs);
  }
  return NULL;
}

void socket_create(int argc , char *argv[]){
  struct sockaddr_un serv_addr, cli_addr;
  
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
    vector_threads = malloc(sizeof(pthread_t)*10);
    fs = new_tecnicofs();
    socket_create(argc, argv);
    exit(EXIT_SUCCESS);
}