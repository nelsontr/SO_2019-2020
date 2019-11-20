//Example code: A simple server side code, which echos back the received message.
//Handle multiple socket connections with select and fd_set on Linux
#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h> //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#define PORT 8080

int main(int argc , char *argv[]){
	int master_socket , addrlen , new_socket , client_socket[30] ,
		max_clients = 30 , activity, i , valread , sd;
	int max_sd;

	fd_set readfds;
	char buffer[1025]; //data buffer of 1K
	struct sockaddr_in address;

	//create a master socket
	master_socket = socket(AF_UNIX, SOCK_STREAM , 0);
	if( master_socket == 0){
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	//type of socket created
	address.sin_family = AF_INET;
	strcpy(address.sin_addr.s_addr, argv[1]);
	address.sin_port = htons( PORT );

	//bind the socket to localhost port 8888
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	//try to specify maximum of 5 pending connections for the master socket
	if (listen(master_socket, 5) < 0){
		perror("listen");
		exit(EXIT_FAILURE);
	}

	//accept the incoming connection
	addrlen = sizeof(address);
	puts("Waiting for connections ...");






	while(1){

    FD_ZERO(&readfds);
    FD_SET(master_socket, &readfds);
		max_sd = master_socket;

    for ( i = 0 ; i < max_clients ; i++){
			sd = client_socket[i];
      if(sd > 0) FD_SET( sd , &readfds);
			if(sd > max_sd) max_sd = sd;
		}

		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

		if (FD_ISSET(master_socket, &readfds)){
			if ((new_socket = accept(master_socket,(struct sockaddr *)&address,
        (socklen_t*)&addrlen))<0){
				perror("accept");
				exit(EXIT_FAILURE);
			}

			for (i = 0; i < max_clients; i++)
				if( client_socket[i] == 0 ){
					client_socket[i] = new_socket;
					printf("Adding to list of sockets as %d\n" , i);
					break;
			  }
    }

		for (i = 0; i < max_clients; i++){
			sd = client_socket[i];
			if (FD_ISSET( sd , &readfds)){
				if ((valread = read( sd , buffer, 1024)) == 0){
          buffer[valread] = '\0';
					send(sd , buffer , strlen(buffer) , 0 );
					close( sd );
					client_socket[i] = 0;
				}
		  }
	  }
  }

	return 0;
}
