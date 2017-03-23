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


#define BufLen 512

struct RWPacket {
	short OpCode;
	char* Filename;
	char* Mode;
	int HostTID;
	int ClientTID;
};

struct DataPacket {
	short Opcode;
	short Block;
	char* Data;
};

struct ACKPacket {
	short Opcode;
	short Block;
};

struct ErrorPacket {
	short Opcode;
	short Code;
	char* ErrorMsg;
};

//WR=0 is a write request WR=1 is a read request



int Child_Process(int pipe_fds, int sock_fd, struct  RWPacket type)//, struct Request_Datagram)
{
	struct DataPacket data;
	struct ACKPacket ack;
	struct ErrorPacket;
	int alive = 1;
	fd_set readfds;
	char buf[BufLen];
	struct timeval tv;
	int nfds = pipe +1;
	int timeoutCount = 0;
	int result = 0;
	char block[512];
	int WR = 0;
	FILE *fp;
	tv.tv_sec = 1;
	while(alive)
	{
		FD_ZERO(&readfds);
		FD_SET(pipe_fds, &readfds);
		result = select(nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
		if(result>0)
		{
			timeoutCount = 0;
			read(pipe_fds, buf, BufLen);
			if(WR == 1)
				// cast buf into tftp struct ACK
				fp = fopen(/*buf.FileName*/"","r");
				//
				
			} else if(WR == 2){
				// cast buf into tftp struct DATAGRAM
			} else if(WR == 3){
			
			} else if(WR == 4){
				
			} else if(WR == 5){
				
			}
			
			
			
		}
		
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

