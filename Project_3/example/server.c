#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h> 
#define MAX 80 
#define PORT 8888 
#define SA struct sockaddr 

pthread_t *producer_th;
int i=0;

// Function designed for chat between client and server. 
void* func(void *args) 
{ 
	int sockfd =*(int*)args;
	char buff[MAX]; 
	int n; 
	// infinite loop for chat 
	for (;;) { 
		bzero(buff, MAX); 

		// read the message from client and copy it in buffer 
		read(sockfd, buff, sizeof(buff)); 
		// print buffer which contains the client contents 
		printf("From client: %s\t To client : ", buff); 
		bzero(buff, MAX); 
		n = 0; 
		// copy server message in the buffer 
		while ((buff[n++] = getchar()) != '\n') 
			; 

		// and send that buffer to client 
		write(sockfd, buff, sizeof(buff)); 

		// if msg contains "Exit" then server exit and chat ended. 
		if (strncmp("exit", buff, 4) == 0) { 
			printf("Server Exit...\n"); 
			break; 
		} 
	} 
	return NULL;
} 


// Driver function 
int main(int argc, char* argv[]) 
{ 
int sockfd, connfd, len; 
	pthread_t* workers = (pthread_t*) malloc(atoi(argv[2]) * sizeof(pthread_t));	
	struct sockaddr_in servaddr, cli; 

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = atoi(argv[1]); 
	servaddr.sin_port = htons(PORT); 

	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n"); 

	while(1){

		// Now server is ready to listen and verification 
		if ((listen(sockfd, 5)) != 0) { 
			printf("Listen failed...\n"); 
			exit(0); 
		} 
		else
			printf("Server listening..\n"); 
		len = sizeof(cli); 

		// Accept the data packet from client and verification 
		connfd = accept(sockfd, (SA*)&cli, &len); 
		if (connfd < 0) { 
			printf("server acccept failed...\n"); 
			exit(0); 
		} 
		else
			printf("server acccept the client...\n"); 
	// Function for chatting between client and server 
	if (pthread_create(&workers[i++], NULL, func, &connfd)!= 0)
		printf("OK");
	
	// After chatting close the socket 
	close(sockfd); 
	}
	
	for(int x = 0; x < i; x++)
        	if(pthread_join(workers[x], NULL))
			printf("PL");
}

