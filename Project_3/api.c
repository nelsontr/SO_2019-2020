/**
 * Sistemas Operativos, DEI/IST/ULisboa 2019-20
 * Created by Matheus and Nelson, group 22
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "tecnicofs-client-api.h"

int sockfd;

int tfsCreate(char *filename, permission ownerPermissions, permission othersPermissions) {
  char buff[MAX_INPUT]="";
  dprintf(sockfd, "c %s %d", filename, (ownerPermissions*10+othersPermissions));
  read(sockfd,&buff,sizeof(buff));
  return atoi(buff);
}

int tfsDelete(char *filename){
  char buff[MAX_INPUT]="";
  dprintf(sockfd, "d %s", filename);
  read(sockfd,&buff,sizeof(buff));
  return atoi(buff);
}

int tfsRename(char *filenameOld, char *filenameNew){
  char buff[MAX_INPUT]="";
  dprintf(sockfd, "r %s %s", filenameOld, filenameNew);
  read(sockfd,&buff,sizeof(buff));
  return atoi(buff);
}

int tfsOpen(char *filename, permission mode){
  char buff[MAX_INPUT]="";
  dprintf(sockfd, "o %s %d", filename, mode);
  read(sockfd,&buff,sizeof(buff));
  return atoi(buff);
}

int tfsClose(int fd){
  char buff[MAX_INPUT]="";
  dprintf(sockfd, "x %d", fd);
  read(sockfd,&buff,sizeof(buff));
  return atoi(buff);
}

int tfsRead(int fd, char *buffer, int len){
  char buff[MAX_INPUT]="";
  dprintf(sockfd, "l %d %d", fd,len);
  read(sockfd,&buff,sizeof(buff));
  buff[MAX_INPUT]=0;
  sscanf(buff, "%s %d", buffer, &len);
  return len;
}

int tfsWrite(int fd, char *buffer, int len){
  char buff[MAX_INPUT]="";
  dprintf(sockfd, "w %d %s", fd,buffer);
  read(sockfd,&buff,sizeof(buff));
  return atoi(buff);
}

int tfsMount(char * address){
	struct sockaddr_un serv_addr;
	
	sockfd = socket(AF_UNIX,SOCK_STREAM,0);
  if (sockfd < 0)
    puts("server: can't open stream socket");

  bzero((char *) &serv_addr, sizeof(serv_addr));

  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, address);
  int servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if(connect(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0){
    puts("client: can't connect to server");
    exit(TECNICOFS_ERROR_CONNECTION_ERROR);
  }
}

int tfsUnmount(){
  dprintf(sockfd, "e");
	close(sockfd);
}