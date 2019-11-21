#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define UNIXSTR_PATH "/tmp/s.unixstr"
#define UNIXDG_PATH "/tmp/s.unixdgx"
#define UNIXDG_TMP "/tmp/dgXXXXXXX"



int main(int argc , char *argv[]){
  struct sockaddr_un serv_addr, cli_addr;
	int sockfd;
	FILE* fout=fopen(argv[2],"w");
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
  int len=sizeof(cli_addr);
		puts("Waiting..");
  int newsockfd = accept(sockfd,(struct sockaddr *) &serv_addr, &len);
    if (newsockfd < 0) puts("server: accept error");
  
  char recvline[10];
  int n = read(newsockfd, recvline, 10);
  recvline[n]=0;
  
  fputs(recvline, fout);
  fclose(fout);
  
}
