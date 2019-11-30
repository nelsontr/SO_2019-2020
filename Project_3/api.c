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

#define MAX_INPUT 150

int sockfd;
char buff[MAX_INPUT];

int tfsCreate(char *filename, permission ownerPermissions, permission othersPermissions) {
  dprintf(sockfd, "c %s %d", filename, (ownerPermissions*10+othersPermissions));
  read(sockfd,&buff,sizeof(buff));
  return atoi(buff);
}

int tfsDelete(char *filename){
  dprintf(sockfd, "d %s", filename);
  read(sockfd,&buff,sizeof(buff));
  return atoi(buff);
}

int tfsRename(char *filenameOld, char *filenameNew){
  dprintf(sockfd, "r %s %s", filenameOld, filenameNew);
  read(sockfd,&buff,sizeof(buff));
  return atoi(buff);
}


int tfsOpen(char *filename, permission mode){
  dprintf(sockfd, "o %s %d", filename, mode);
  read(sockfd,&buff,sizeof(buff));
  return atoi(buff);
}

int tfsClose(int fd){
  dprintf(sockfd, "x %d", fd);
  read(sockfd,&buff,sizeof(buff));
  return atoi(buff);
}

int tfsRead(int fd, char *buffer, int len){
  dprintf(sockfd, "l %d %d", fd,len);
  read(sockfd,&buff,sizeof(buff));
  len = strlen(buff)-2;
  buff[len]=0;
  strcpy(buffer,buff);
  printf("%d\n",len);
  //read(sockfd,&buff,sizeof(buff));
  return len;
}

int tfsWrite(int fd, char *buffer, int len){
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


/*int main(int argc, char* argv[]){
  char name[10];
  tfsMount(argv[1]);
  enum permission owner = RW;
  enum permission outro = READ;
  tfsCreate("abc", owner, outro);
  int fd = tfsOpen("abc",RW);
  printf(">>%d\n",fd);
  tfsWrite(fd,"12345", 5);
  tfsCreate("bc", owner, outro);
  tfsRead(fd, name, 5);
  printf("\n%s\n", name);
  tfsUnmount();
}*/