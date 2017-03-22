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


int Child_Process(int pipe_fds, int sock_fd)
{
	TFTP *Packet
	nbytes = read
	return 1;
}

//Function Prototypes
int MakeSocket(int port);



int main (int arc, char** argv)

{
	int port = 69; // This is the fort defined or tFTP
	
	int socketFD = MakeSocket(port);
	
	
	return 1;
}





//This fuction creates the socket for the server, and binds it to port
//Most cases this will be port 69 for this homework
int MakeSocket(int port)
{
	int domain, type, protocol, serverFD;
	
	domain = AF_INET;
	type = SOCK_DGRAM;
	protocol = 0;
	
	serverFD = socket(domain, type, protocol);
	
	struct sockaddr_in address;
	
	bzero(&address, sizeof(struct sockaddr_in));
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);
	
	if(bind(serverFD, (struct sockaddr *) &address, sizeof(address))==-1)
	{
		printf("Bind() Error\n");
		exit(EXIT_FAILURE);
	}
	
	return serverFD;
	
	
}

