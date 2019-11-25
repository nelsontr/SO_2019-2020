#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/un.h>

int main(int argc, char* argv[]) {
    int sockfd, servlen, len, newsockfd;
    struct sockaddr_un serv_addr, cli_addr;
    sockfd = socket(AF_UNIX,SOCK_STREAM,0);
  if (sockfd < 0)
    puts("server: can't open stream socket");
    
  unlink(argv[1]);

  bzero((char *)&serv_addr, sizeof(serv_addr));

  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, argv[1]);
  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
  
  if (bind(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
    puts("server, can't bind local address");

  listen(sockfd, 5);
  len=sizeof(cli_addr);
		puts("Waiting..");
  newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &len);
    if (newsockfd < 0) puts("server: accept error");
}

