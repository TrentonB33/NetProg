// I'm a file yay
#include <stdio.h>			// For stdout, stderr
#include <string.h>			// For strlen(), strcpy(), bzero()
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>

#define MAXSIZE 516



#define BufLen 512

//Function Prototypes
int MakeSocket(uint16_t* port);
short ParsePacket(char* buf, void* result);
int RunServer(int sockFD);
void CheckChildren(pid_t* children, int* curSize);
int compare (const void * a, const void * b);
void SendErrorPacket(int socketFD, int EC, char* message, struct sockaddr_in* dest);


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
	char Data[512];
};

struct ACKPacket {
	short OpCode;
	short Block;
};

struct ErrorPacket {
	short OpCode;
	short ErrCode;
	char ErrorMsg[512];
};

//WR=0 is a write request WR=1 is a read request




int Child_Process( struct sockaddr_in * dest, struct RWPacket* type)//, struct Request_Datagram)
{
	struct DataPacket* data;
	struct ACKPacket* ack = malloc(sizeof(char)*4);;
	struct ErrorPacket* err;
	char FDNE[] = "File Does not Exist\0";
	char FAE[] = "File Already Exists";
	uint16_t port = 0; 
	int alive = 1;
	
	fd_set readfds;
	char buf[MAXSIZE];
	struct timeval tv;
	int timeoutCount = 0;
	int result = 0;
	char block[BufLen];
	
	short WR = type->OpCode;
	void* recv = NULL;
	int opCode, blockNum, errsv, written, read;
	FILE * fp;
	
	int socketFD = MakeSocket(&port);
	int nfds = socketFD+1;
	struct sockaddr_in clientAddr;
	socklen_t addrLen = sizeof(struct sockaddr_in);
	
	blockNum = 0;
	tv.tv_sec = 1;
	bzero(buf, BufLen);
	//perror("Nut\n");
	if(WR == 1)
	{
		// cast buf into tftp struct ACK
		blockNum = 1;
		opCode = 3;
		fp = fopen(/*buf.FileName*/type->Filename, "r");
		errsv = errno;
		if(errsv == 2)
		{
			err =  malloc(sizeof(char)*512);
			err->OpCode = htons(5);
			err->ErrCode = htons(1);
			//err->ErrorMsg = "File Does not Exist";
			strcpy(err->ErrorMsg, FDNE);
			sendto(socketFD, err, 512, 0, (struct sockaddr*)dest, sizeof(struct sockaddr_in));
			//free(err);
			//free(type);
			return 1;
		} //else if(
		read = fread(&block, 1, BufLen, fp);
		//block[read] = '\0';
		data = malloc(sizeof(char)*516);
		data->OpCode = htons(3);
		data->Block = htons(blockNum);
		//data->Data = block;
		memcpy(data->Data, block, 512);
		written = sendto(socketFD, data,read+4,0, (struct sockaddr*)dest,sizeof(struct sockaddr_in));
		blockNum++;
		//
	}  else if(WR == 2){
		opCode = 4;
		fp = fopen(/*buf.FileName*/type->Filename, "r");
		errsv = errno;
		if(fp != NULL)
		{
			//printf("errloop\n");
			err =  malloc(sizeof(char)*512);
			err->OpCode = htons(5);
			err->ErrCode = htons(6);
			strcpy(err->ErrorMsg, FAE);
			sendto(socketFD, err, 512, 0, (struct sockaddr*)dest, sizeof(struct sockaddr_in));
			/*free(err);
			free(type);
			fclose(fp);*/
			return 1;
		}
		fp = fopen(type->Filename, "w");
		ack = malloc(sizeof(char)*4);
		ack->OpCode = htons(4);
		ack->Block = htons(blockNum);
		sendto(socketFD, ack,516,0, (struct sockaddr*)dest,sizeof(struct sockaddr_in));
		//printf("Sent!\n");
		blockNum = 1;
		// cast buf into tftp struct DATAGRAM
	}
	while(alive  && timeoutCount<11)
	{
		//usleep(10);
		//printf("HI! %d, %d\n", timeoutCount, blockNum);
		bzero(buf, BufLen);
		FD_ZERO(&readfds);
		FD_SET(socketFD, &readfds);
		tv.tv_sec = 1;
		result = select(nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
		//printf("maybe\n");
		if(result>0 || blockNum == 0)
		{
			//printf("here!\n");
			timeoutCount = 0;
			read = recvfrom(socketFD, buf, MAXSIZE, 0, (struct sockaddr*) &clientAddr, &addrLen);
			if(recv!=NULL && ntohs(((struct DataPacket*) recv)->OpCode) == 3)
			{
				//printf("break\n");//free(((struct DataPacket*)recv)->Data);
				free(recv);
			}
			recv  = calloc(1,sizeof(struct DataPacket));
			opCode = ParsePacket(buf, recv);
			//printf("hey %d\n", ntohs(((struct DataPacket*) recv)->OpCode));
			if(clientAddr.sin_port != dest->sin_port)
			{
				err =  malloc(sizeof(char)*512);
				err->OpCode = htons(5);
				err->ErrCode = htons(6);
				strcpy(err->ErrorMsg, FDNE);
				sendto(socketFD, err, 512, 0, (struct sockaddr*)&clientAddr, sizeof(struct sockaddr_in));
				continue;
			}
			
			if(opCode == 3){
				data = recv;
				//printf("put %d\n", read);
				if(ntohs(data->Block) == blockNum)
				{
					written = fwrite(data->Data, 1, read-4, fp);
					//printf("write:  %d\n", written);					
					ack->OpCode = htons(4);
					ack->Block = htons(blockNum);
					sendto(socketFD, ack, 4,0, (struct sockaddr*)dest,sizeof(struct sockaddr_in));
					if(written < 512)
					{
						//printf("dead\n");
						/*free(data);
						free(ack);
						fclose(fp);
						close(socketFD);
						free(dest);
						free(type);*/
						return 0;
					}
					blockNum++;
				}

				
			} else if(opCode == 4){
				if(written<512)
				{
					/*free(data);
					free(ack);
					fclose(fp);
					close(socketFD);
					free(dest);
					free(type);*/
					return 0;
				}
				free(data);
				read = fread(&block, 1, BufLen, fp);
				//printf("Read: %d", read);
				data = malloc(sizeof(char)*516);
				data->OpCode = htons(3);
				data->Block = htons(blockNum);
				//data->Data = block;
				memcpy(data->Data, block, 512);
				written = sendto(socketFD, data, read+4,0, (struct sockaddr*)dest,sizeof(struct sockaddr_in));
				blockNum++;
				
			} else if(opCode == 5){
				free(data);
				free(ack);
				fclose(fp);
				free(dest);
				free(type);
				close(socketFD);
				return 1;
			} else {
				
				SendErrorPacket(socketFD, 4, "Unknown TFTP operation", dest);
				
			}
				
		} else {
			if(WR == 1)
			{
				sendto(socketFD, data,read+4,0, (struct sockaddr*)dest,sizeof(struct sockaddr_in));
			} else if(WR == 2)
			{
				sendto(socketFD, ack, 4,0, (struct sockaddr*)dest,sizeof(struct sockaddr_in));
			}
		}
		timeoutCount++;
			
	}
		
	return 1;
}



int main (int arc, char** argv)
{
	uint16_t port = 0; 	
	int socketFD = MakeSocket(&port);
	
	printf("%d\n", (int)port);
	
	RunServer(socketFD);
	
	return EXIT_SUCCESS;
}

//Runs the server
int RunServer(int sockFD)
{
	struct sockaddr_in* clientAddr = (struct sockaddr_in*)calloc(1,sizeof(struct sockaddr_in));
	socklen_t addrLen = sizeof(struct sockaddr_in);
	
	int amtRead = 0;
	char message[MAXSIZE];
	
	int opCode = 0;
	void* result = calloc(1, sizeof(char));
	
	int maxChildren = 2;
	int curChildren = 0;
	pid_t* childProcs = (pid_t*)calloc(maxChildren, sizeof(pid_t));
	pid_t childID = 0;
	
	
	int running = 1;
	
	while(running)
	{
				
		bzero(message, MAXSIZE);
		bzero(clientAddr, sizeof(struct sockaddr_in));

		amtRead = recvfrom(sockFD, message, MAXSIZE, 0, (struct sockaddr *) clientAddr, &addrLen);

		//printf("Got a message from: %d:%d\n", ntohl(clientAddr->sin_addr.s_addr), 
		//	   ntohs(clientAddr->sin_port));

		//printf("Size: %d\n", amtRead);
		//write(1, message, amtRead);
		//printf("\n");
		
		opCode = ParsePacket(message, result);
		//printf("Opcode:  %d\n", opCode);
		if(opCode < 3)
		{
			
			childID = fork();
			if(childID == 0)
			{
				//printf("I'm a child!!   %d\n", ((struct RWPacket*)result)->OpCode);
				Child_Process (clientAddr, (struct RWPacket *) result);
				//remember to pipe to the parent that the process is over, or research
				//again how the parent knows the child ended.
			}
			else if(childID > 0)
			{
				if(curChildren == maxChildren)
				{
					
					pid_t* temp = (pid_t*) calloc(maxChildren, sizeof(pid_t));
					memcpy(temp, childProcs, maxChildren);
					maxChildren = maxChildren * 2;
					childProcs = (pid_t*) calloc(maxChildren, sizeof(pid_t));
					memcpy(childProcs,temp, maxChildren/2);	
					
				}
				
				curChildren++;
				childProcs[curChildren] = childID;
			}
		}
		else
		{
			SendErrorPacket(sockFD, 0, 
				"Cannot send requests other than read and write to the server.",
				clientAddr);
		}
		
		CheckChildren(childProcs, &curChildren);
		
		
	}
	
	free(clientAddr);
	
	return EXIT_SUCCESS;
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
	
	if(*port == 0)
		address.sin_port = htons(0);
	else
		address.sin_port = htons(*port);
	
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
		printf("Errno: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	
	*port = ntohs(tempAdr.sin_port);

	return serverFD;
}

//Pass the destination variable in as result(an empty void*) and the packet in as buf.  The function places the resulting packet struct in result and returns the ID of the struct
// Make sure to cast the reuslt to a pointer to the correct Packet type.
short ParsePacket(char* buf, void* result)
{
	short opCode = 0;
	size_t codeLen = 2;
	struct RWPacket* rw;
	struct DataPacker* dp;
	struct ACKPacket* ap;
	struct ErrorPacket* ep;
	char * errMsg;
	memcpy(&opCode, buf, codeLen);
	//printf("-------------------------------------------%d    %d\n", opCode,ntohs(opCode));
	opCode = ntohs(opCode);
	
	if(opCode == 1 || opCode == 2)
	{
		rw = calloc(1, sizeof(char)*512);
		rw->OpCode = opCode;
		//printf("%d\n", rw->OpCode);
		rw->Filename = strdup(buf+2);
		memcpy(result, rw,512);
		//free(rw);
		
	} else if(opCode == 3)
	{
		dp = malloc(sizeof(char)*516);
		memcpy(dp,buf,516);
		memcpy(result, dp, 516);
		//free(dp);
		
	} else if(opCode == 4)	
	{
		ap = malloc(sizeof(char)*4);
		memcpy(ap,buf,4);
		memcpy(result, ap, 4);
		//free(ap);
		
	} else if(opCode == 5)
	{
		ep = malloc(sizeof(char)*512);
		memcpy(ep,buf,4);
		errMsg = strdup(buf+4);
		strcpy(ep->ErrorMsg, errMsg);
		memcpy(result, ep, 512);
		//free(ep);
		
	}
	
	
	
	//printf("--------+++++++++ %d",opCode);
	return opCode;
}


//Checks to see if a child process has terminated and modifies the params as needed
void CheckChildren(pid_t* children, int* curSize)
{
	int origSize = *curSize;
	int* status = NULL;
	int itr = 0;
	pid_t retVal = 0;
	
	while(itr < origSize)
	{
		retVal = waitpid(children[itr], status, WNOHANG);
		if(retVal == -1)
		{
			printf("waitpid Error\n");
			printf("Errno: %d\n", errno);
			exit(EXIT_FAILURE);
		}
		else
		{
			if(WIFEXITED(status))
			{
				//printf("Child finished successfully\n");
				children[itr] = 0;
					  
				*curSize = (*curSize) -1;
				
			}
			else
			{
				//printf("Child died a horrible death\n");
			}
		}
		itr++;
	}
	
	//printf("Current Size: %d\n", *curSize);
	qsort(children, origSize, sizeof(pid_t), compare);
}

//Comparator function for qsort
int compare (const void * a, const void * b)
{
  return ( *(int*)b - *(int*)a );
}

//Send an error packet
void SendErrorPacket(int socketFD, int EC, char* message, struct sockaddr_in* dest)
{
	struct ErrorPacket* err;
	
	err =  (struct ErrorPacket*)calloc(1,sizeof(struct ErrorPacket));
	err->OpCode = htons(5);
	err->ErrCode = htons(EC);
	strcpy(err->ErrorMsg, message);
	sendto(socketFD, err, MAXSIZE, 0, (struct sockaddr*)dest, sizeof(struct sockaddr_in));
	
}















