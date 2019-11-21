#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define UNIXSTR_PATH "/tmp/s.unixstr"
#define UNIXDG_PATH "/tmp/s.unixdgx"
#define UNIXDG_TMP "/tmp/dgXXXXXXX"



int mount(int argc , char *argv[]){
  struct sockaddr_un serv_addr, cli_addr;
	int sockfd;

  if ((sockfd = socket(AF_UNIX,SOCK_STREAM,0) ) < 0)
    puts("server: can't open stream socket");

  bzero((char *) &serv_addr, sizeof(serv_addr));

  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, argv[1]);

  int servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if(connect(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
		puts("client: can't connect to server");
		
	char buf[10];
	
	read(STDIN_FILENO, buf, sizeof(buf));
  write(sockfd, buf, sizeof(buf));
  return 0;
}



int main(int argc , char *argv[]){
	mount(argc,argv);
	return 0;
}
