// I'm a file yay
#include <stdio.h>			// For stdout, stderr
#include <string.h>			// For strlen(), strcpy(), bzero()
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <dirent.h>

#define MAXSIZE 516



#define BufLen 512

//Function Prototypes
int MakeSocket(uint16_t* port);
void PopulateFiles(char** files, char* serverDir);
int ParsePacket(char* buf, void* result);

struct RWPacket {
	short OpCode;
	char* Filename;
	char* Mode;
	int HostTID;
	int ClientTID;
};

struct DataPacket {
	short OpCode;
	short Block;
	char* Data;
};

struct ACKPacket {
	short OpCode;
	short Block;
};

struct ErrorPacket {
	short OpCode;
	short ErrCode;
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
	short WR = type.OpCode;
	void* recv;
	FILE *fp;
	tv.tv_sec = 1;
	if(WR == 1)
		// cast buf into tftp struct ACK
		fp = fopen(/*buf.FileName*/"","r");
		//
	}  else if(WR == 2){
		// cast buf into tftp struct DATAGRAM
	}
	while(alive)
	{
		FD_ZERO(&readfds);
		FD_SET(pipe_fds, &readfds);
		result = select(nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
		if(result>0)
		{
			timeoutCount = 0;
			read(pipe_fds, buf, BufLen);
			ParsePacket(
			
				
			} else if(WR == 3){
			
			} else if(WR == 4){
				
			} else if(WR == 5){
				
			}
			
			
			
		}
		
	return 1;
}



int main (int arc, char** argv)

{
	uint16_t port = 0; 
	int amtRead = 0;
	
	char fileDir[] = "ServerFiles";
	char** curFiles = NULL;
	
	int socketFD = MakeSocket(&port);
	printf("%d\n", (int)port);
	
	PopulateFiles(curFiles, fileDir);
	
	
	struct sockaddr_in clientAddr;
	socklen_t addrLen = 0;
	
	char message[MAXSIZE];
	bzero(message, MAXSIZE);
	bzero(&clientAddr, sizeof(struct sockaddr_in));
	
	amtRead = recvfrom(socketFD, message, MAXSIZE, 0, (struct sockaddr *) &clientAddr, &addrLen);
	
	printf("Got a message from: %il:%i\n", clientAddr.sin_addr.s_addr, clientAddr.sin_port);
	printf("Size: %d\n", amtRead);
	write(1, message, amtRead);
	printf("\n");

	
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


void PopulateFiles(char** files, char* serverDir)
{
	
	DIR* fileDir;
	struct dirent* item;
	
	if((fileDir = opendir(serverDir)) != NULL)
	{
		
	}
	else
	{
		if(mkdir(serverDir, 0x666) == -1)
		{
			printf("mkdir Error\n");
			printf("Errno: %d\n", errno);
			exit(EXIT_FAILURE);
		}
	}

	
}

//Pass the destination variable in as result(an empty void*) and the packet in as buf.  The function places the resulting packet struct in result and returns the ID of the struct
// Make sure to cast the reuslt to a pointer to the correct Packet type.
int ParsePacket(char* buf, void* result)
{
	short * opCode = malloc(sizeof(short));
	size_t codeLen = 2;
	struct RWPacket* rw;
	struct DataPacker* dp;
	struct ACKPacket* ap;
	struct ErrorPacket* ep;
	memcpy(opCode, buf, codeLen);
	if(*opCode == 1 || *opCode == 2)
	{
		rw = malloc(sizeof(char)*512);
		rw->OpCode = *opCode;
		rw->Filename = strdup(buf+2);
		result = rw;
		
	} else if(*opCode == 3)
	{
		dp = malloc(sizeof(char)*516);
		memcpy(dp,buf,516);
		result = dp;
		
	} else if(*opCode == 4)	
	{
		ap = malloc(sizeof(char)*4);
		memcpy(ap,buf,4);
		result = ap;
		
	} else if(*opCode == 5)
	{
		ep = malloc(sizeof(char)*512);
		memcpy(ep,buf,4);
		ep->ErrorMsg = strdup(buf+4)
		result = ep;
		
	}
	return*opCode;
}









