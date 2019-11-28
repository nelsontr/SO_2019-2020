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
  int iNumber=0, op=0;
  char token,name[MAX_INPUT_SIZE];
  sscanf(buff, "%s %s", &token, name);
  switch (token) {
      case 'c':
          lookup(fs,name);
          if (lookup(fs,name)==0){
            iNumber = obtainNewInumber(fs);	        
            create(fs, name, iNumber,0);
            op=2;
            }
          else
            op=1;
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
      default: {  error 
          mutex_unlock(&commandsLock);
          fprintf(stderr, "Error: commands to apply\n");
          exit(EXIT_FAILURE);
      }*/
  }
  dprintf(userid,"%d",op);
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