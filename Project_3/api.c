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

#define UNIXSTR_PATH "/tmp/s.unixstr"
#define UNIXDG_PATH "/tmp/s.unixdgx"
#define UNIXDG_TMP "/tmp/dgXXXXXXX"


int sockfd;

int tfsCreate(char *filename, permission ownerPermissions, permission othersPermissions){
  char buff[10];
  dprintf(sockfd, "c %s", filename);
  read(sockfd, buff, 10);
}

int tfsDelete(char *filename){
  dprintf(sockfd, "d %s", filename);
  
}

int tfsRename(char *filenameOld, char *filenameNew){
  dprintf(sockfd, "r %s %s", filenameOld, filenameNew);
}


int tfsOpen(char *filename, permission mode);
int tfsClose(int fd);
int tfsRead(int fd, char *buffer, int len);
int tfsWrite(int fd, char *buffer, int len);


int tfsMount(char * address){
	struct sockaddr_un serv_addr;
	
	sockfd = socket(AF_UNIX,SOCK_STREAM,0);
  if (sockfd < 0)
    puts("server: can't open stream socket");

  bzero((char *) &serv_addr, sizeof(serv_addr));

  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, address);
  int servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if(connect(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
		puts("client: can't connect to server");
}


int tfsUnmount(){
	close(sockfd);
}


int main(int argc, char* argv[]){
  tfsMount(argv[1]);
  tfsCreate("abc", 0, 0);
  tfsUnmount();
}