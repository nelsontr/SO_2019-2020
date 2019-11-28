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


#define UNIXSTR_PATH "/tmp/s.unixstr"
#define UNIXDG_PATH "/tmp/s.unixdgx"
#define UNIXDG_TMP "/tmp/dgXXXXXXX"
#define MAX 150

int sockfd, newsockfd;
pthread_t *vector_threads;
tecnicofs *fs;

void* applyComands(void *args){
  int userid = *(int*) args;
  char buff[MAX];
	bzero(buff, MAX); 
  int n = sizeof(buff);

  while(1){
	read(userid, buff, n); 
  buff[n]=0;
  int iNumber=0, op;
  char token,name[MAX_INPUT_SIZE];
  int owner,others;
  sscanf(buff, "%s %s", &token, name);
  switch (token) {
      case 'c': 
        if (lookup(fs,name)==0){
          sscanf(buff, "%s %s %d%d", &token, name, &owner, &others);
          iNumber = inode_create(userid,owner,others);	        
          create(fs, name, iNumber,0);
          dprintf(userid,"%d",0);
        }
        else dprintf(userid,"%d", -4);
        break;
      /*case 'l':
          mutex_unlock(&commandsLock);
          sem_post_err(&sem_prod,"Producer");		        
          int searchResult = lookup(fs, name);
          if(!searchResult)
              printf("%s not found\n", name);
          else
              printf("%s found with inumber %d\n", name, searchResult);
          break;
      */case 'd':
          if ((iNumber=lookup(fs,name))!=0){
            delete(fs, name,0);
          }
          break;/*
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
      default: {  error 
          mutex_unlock(&commandsLock);
          fprintf(stderr, "Error: commands to apply\n");
          exit(EXIT_FAILURE);
      }*/
  }
  
  print_tecnicofs_tree(stdout,fs);
  //printf("%s\n", buff); 
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