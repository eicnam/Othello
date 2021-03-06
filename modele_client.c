#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#define SERVEUR "127.0.0.1"

static void stopChild(int signo);

int port;
int descGuiToClient;        

int main(int argc, char **argv)
{
	int sockfd, new_fd, rv, sin_size, numbytes;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr their_adr;
	char buf[100];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	rv = getaddrinfo(SERVEUR, argv[1], &hints, &servinfo);

	port = atoi(argv[2]);

	if (signal(SIGTERM, stopChild) == SIG_ERR) {
		printf("Could not attach signal handler\n");
		return EXIT_FAILURE;
	}

	if(rv != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}


	// Creating socket and linking
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("client: socket");
			continue;
		}
		if((connect(sockfd, p->ai_addr, p->ai_addrlen) == -1))
		{
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if(p == NULL)
	{
		fprintf(stderr, "client: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); 	// free struct

	char portInChar[6]; 
	sprintf(portInChar, "%d", port);

	char message[30];
	strcpy(message, "demande,");
	strcat(message, portInChar);

	// if it is a demand, send this ...
	if (strcmp(argv[3], "0")==0)
	{
		send(sockfd, message, strlen(message), 0);
	}
	// else send an acknoledgment
	else if (strcmp(argv[3], "1")==0)
	{
		send(sockfd, "ack", 3, 0);
	}

	char guiToClient[] = "guiToClient.fifo";

	if((descGuiToClient= open(guiToClient, O_RDONLY)) == -1) 
	{   
		fprintf(stderr, "Unable to open the exit of the named pipe.\n");
		exit(EXIT_FAILURE);
	}   

	// we now obey to the pipe messages
	char stringToRead[7];
	int nbBRead;
	while(1)
	{
		if((nbBRead = read(descGuiToClient, stringToRead, 7-1)) == -1)
		{
			perror("read error : ");
			exit(EXIT_FAILURE);
		}else if(nbBRead > 0)
		{
			stringToRead[nbBRead] = '\0';
			send(sockfd, stringToRead, strlen(stringToRead), 0);
		}
	}

	close(sockfd);
	exit(0);
}

static void stopChild(int signo)
{
	printf("Client : Closing client...\n");
	fflush(stdout);

	int res;
	if((res = close(descGuiToClient))==-1)
	{
		perror("Server : close");
		exit(EXIT_FAILURE);
	}

	printf("Client : Stopped\n");
	fflush(stdout);

	exit(0);
}
