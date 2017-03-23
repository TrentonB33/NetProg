// I'm a file yay
#include <stdio.h>			// For stdout, stderr
#include <string.h>			// For strlen(), strcpy(), bzero()
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

#define MAXSIZE 516


//Function Prototypes
int MakeSocket(uint16_t* port);



int main (int arc, char** argv)
{
	uint16_t port = 0; 
	
	int socketFD = MakeSocket(&port);
	printf("%d\n", (int)port);
	
	
	struct sockaddr_in clientAddr;
	socklen_t addrLen = 0;
	
	char message[MAXSIZE];
	bzero(message, MAXSIZE);
	bzero(&clientAddr, sizeof(struct sockaddr_in));
	
	recvfrom(socketFD, message, MAXSIZE, 0, (struct sockaddr *) &clientAddr, &addrLen);
	
	printf("Got a message from: %il:%i\n", clientAddr.sin_addr.s_addr, clientAddr.sin_port);

	
	return 1;
}


//This fuction creates the socket for the server, and binds it to port
//Most cases this will be port 69 for this homework
int MakeSocket(uint16_t* port)
{
	int domain, type, protocol, serverFD;
	struct sockaddr_in address, tempAdr;
	
	domain = AF_INET;
	type = SOCK_DGRAM;
	protocol = 0;
	
	serverFD = socket(domain, type, protocol);
	
	bzero(&address, sizeof(struct sockaddr_in));
	bzero(&tempAdr, sizeof(struct sockaddr_in));
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(0);
	
	if(bind(serverFD, (struct sockaddr *) &address, sizeof(struct sockaddr_in))==-1)
	{
		printf("Bind() Error\n");
		printf("Errno: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	
	socklen_t tempsize = sizeof(tempAdr);
	if(getsockname(serverFD, (struct sockaddr*)&tempAdr, &tempsize))
	{
		printf("getsockname() broke\n");
	}
	
	*port = ntohs(tempAdr.sin_port);
	
	
	
	return serverFD;
	
	
}