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
#include <arpa/inet.h>

#define MAXSIZE 516



#define BufLen 512



//OpCode 1/2
struct RWPacket {
	short OpCode;
	char* Filename;
	char* Mode;
	int HostTID;
	int ClientTID;
};

//OpCode 3
struct DataPacket {
	short OpCode;
	short Block;
	char Data[512];
};

//OpCode 4
struct ACKPacket {
	short OpCode;
	short Block;
};

//OpCode 5
struct ErrorPacket {
	short OpCode;
	short ErrCode;
	char ErrorMsg[512];
};

//OpCode 6
struct ContentPacket {
	short OpCode;
	int HostTID;	
	int ClientTID;
};

//OpCode 7
struct QueryPacket {
	short OpCode;
	struct timespec timestamp;
	char Filename[255];
};


struct Contents {
	char** Filenames;
	char** Hashes;
	int count;
};



//Function Prototypes
int MakeSocket(uint16_t* port);
short ParsePacket(char* buf, void* result);
int RunServer(int sockFD);
void CheckChildren(pid_t* children, int* curSize);
int compare (const void * a, const void * b);
void SendErrorPacket(int socketFD, int EC, char* message, struct sockaddr_in* dest);
int Run_Client(int sockFD, int TID);
int Get_Contents(struct sockaddr_in* serverAddr, int TID, int sockFD);
int Child_Process(int socketFD, struct sockaddr_in * dest, struct RWPacket* type);
struct Contents* ReadContents(char* file);
char** CompareContents(int* _gets, int* _puts, int* qct);
char* SetupFiles(char* tempDir, char* hashFile, int* hashFD);
void MakeContentRequest(char* hashFile, void* result);


static struct Contents* _servContents;
static struct Contents* _locContents;

//WR=0 is a write request WR=1 is a read request


void strip_char(char *str, char strip)
{
    char *p, *q;
    for (q = p = str; *p; p++)
        if (*p != strip)
            *q++ = *p;
    *q = '\0';
}

char** CompareContents(int* _gets, int* _puts, int* qct)
{
	int* gets = _gets, * puts = _puts, queryCt = 0, queryarr = 15;
	char** queries = calloc(15, sizeof(char*));
	int x = 0, y = 0;
	for(x=0;x<_locContents->count;x++)
	{
		for(y=0;y<_servContents->count;y++)
		{
			if(strcmp(_locContents->Filenames[x], _servContents->Filenames[y]) == 0)
			{
				gets[y] = 1;
				puts[x] = 1;
				if(strcmp(_locContents->Hashes[x], _servContents->Hashes[y]) != 0)
				{
					queries[queryCt] = _locContents->Filenames[x];
					queryCt++;
					if(queryCt>=queryarr-2)
					{
						queryarr= queryarr * 2;
						queries = realloc(queries, queryarr);
					}
				}
				break;
			}
		}
	}
	*qct = queryCt;
	return queries;
}


int ProcessClientQueries(char ** queries, int queryCt, struct sockaddr_in serverAddr, int sockFD)
{
	struct QueryPacket * query;
	struct stat* buf;
	
	for(int x=0; x<queryCt;x++)
	{
		query = calloc(1, sizeof(struct QueryPacket));
		query->OpCode = 7;
		stat(queries[x], buf);
		query->timestamp = buf->st_mtim;
		memcpy(query->Filename, queries[x], strlen(queries[x]));
	}
}


int Run_Client(int sockFD, int TID)
{
	struct sockaddr_in* serverAddr = (struct sockaddr_in*)calloc(1,sizeof(struct sockaddr_in));
	socklen_t addrLen = sizeof(struct sockaddr_in);
	char ** diffs;
	int* gets, *puts, queries = 0;
	serverAddr->sin_family = AF_INET;
	serverAddr->sin_port = htons(TID);
	serverAddr->sin_addr.s_addr = inet_addr("127.0.0.1");
	
	
	Get_Contents(serverAddr, TID, sockFD);
	
	
	//Creates local hashes ---------------------------
	gets = calloc(_servContents->count, sizeof(int));
	puts = calloc(_locContents->count, sizeof(int));
	memset(gets, 0, _servContents->count);
	memset(puts, 0, _locContents->count);
	diffs = CompareContents(gets, puts, &queries);
	printf("Count: %d\n", queries);
	
	return 0;
}



int Get_Contents(struct sockaddr_in* serverAddr, int TID, int sockFD)
{
	struct ContentPacket* init = calloc(1, sizeof(struct ContentPacket));
	int sentAmt = 0, read, res = 0;
	char buf[MAXSIZE];
	struct RWPacket* rw = calloc(1, sizeof(struct RWPacket));
	socklen_t addrLen = sizeof(struct sockaddr_in);
	init->ClientTID = TID;
	init->OpCode = htons(6);  //Contents OpCode
	
	printf("In Get_Contents.\n");
	printf("Server Address: %d    Port: %d    Socket FD: %d\n", 
		   ntohl(serverAddr->sin_addr.s_addr), ntohs(serverAddr->sin_port), sockFD);
	
	printf("Errno before the send: %d\n", errno);
	
	sentAmt = sendto(sockFD, init, sizeof(struct ContentPacket), 0, (struct sockaddr*)serverAddr,sizeof(struct sockaddr_in)); //Send initial contents packet
	if(sentAmt < 0)
	{
		printf("Sendto broke ya fool.\n Errno: %d\n", errno);
		
	}
	read = recvfrom(sockFD, buf, MAXSIZE, 0, (struct sockaddr*) serverAddr, &addrLen);
	
	ParsePacket(buf, rw);
	
	res = Child_Process(sockFD, serverAddr, rw);
	if(res!=0)
	{
		printf("ERROR: TIMEOUT");
		exit(1);
	} 
	
	_servContents = ReadContents(rw->Filename);
	
	
		
	return 0;
}


struct Contents* ReadContents(char* file)
{
	//perror("hi");
	int count = 0, arrsize = 100;
	FILE * fp;
	char * readBuf = calloc(300, sizeof(char)), * hash, *filename;
	struct Contents* content = calloc(1, sizeof(struct Contents));
	content->Filenames = calloc(arrsize, sizeof(char*));
	content->Hashes = calloc(arrsize, sizeof(char*));
	fp = fopen(/*buf.FileName*/file, "r");
	while(fgets(readBuf, 299, fp))
	{
		
		//printf("wop: %s", readBuf);
		hash = strtok(readBuf, "&");
		
		filename = strtok(NULL, "\r");
		strip_char(filename, '\n');
		printf("Hash: %s    Filename: %s\n", hash, filename);
		content->Hashes[count] = hash;
		content->Filenames[count] = filename;
		//printf("bloop: %s\n",content->Hashes[count]);
		count++;
		if(count>=arrsize-2)
		{
			arrsize = arrsize*2;
			content->Filenames = realloc(content->Filenames, arrsize);
			content->Hashes = realloc(content->Hashes, arrsize);
		}
		readBuf = calloc(300, sizeof(char));
	}
	content->count = count;
	
	return content;
}



int Child_Process(int socketFD, struct sockaddr_in * dest, struct RWPacket* type)//, struct Request_Datagram)
{
	struct DataPacket* data;
	struct ACKPacket* ack = malloc(sizeof(char)*4);;
	struct ErrorPacket* err;
	char FDNE[] = "File Does not Exist\0";
	char FAE[] = "File Already Exists";
	//uint16_t port = 0; 
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
				printf("put %d\n", read);
				if(ntohs(data->Block) == blockNum)
				{
					written = fwrite(data->Data, 1, read-4, fp);
					printf("write:  %d\n", written);					
					ack->OpCode = htons(4);
					ack->Block = htons(blockNum);
					sendto(socketFD, ack, 4,0, (struct sockaddr*)dest,sizeof(struct sockaddr_in));
					if(written < 512)
					{
						fclose(fp);
						//printf("dead\n");
						/*free(data);
						free(ack);
						
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
	uint16_t port = atoi(argv[2]); 	
	int socketFD;
	int* gets, *puts, queries = 0;
	char ** diffs;
	
	_servContents = ReadContents("Butts");
	_locContents = ReadContents("Breasts.txt");
	gets = calloc(_servContents->count, sizeof(int));
	puts = calloc(_locContents->count, sizeof(int));
	memset(gets, 0, _servContents->count * sizeof(int));
	memset(puts, 0, _locContents->count * sizeof(int));
	diffs = CompareContents(gets,puts, &queries);
	printf("Count: %d\n", queries);
	
	for(int x = 0; x <_servContents->count; x++)
	{
		printf("Get %d: %d\n", x, gets[x]);
	}
	
	for(int x = 0; x <queries; x++)
	{
		printf("Query %d: %s\n", x, diffs[x]);
	}
	//printf("Test--\nCount: %d\nHash: %s\nFileName: %s\n",_servContents->count, _servContents->Hashes[0], _servContents->Filenames[0]);
	
	
	
	
	printf("%d\n", (int)port);
	if(strcmp(argv[1],"server") == 0)
	{
		socketFD = MakeSocket(&port);
		RunServer(socketFD);
	}
		
	if(strcmp(argv[1],"client") == 0)
	{
		uint16_t clPort = 0;
		socketFD = MakeSocket(&clPort);
		Run_Client(socketFD, port);
	}
	
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
	void* result = calloc(MAXSIZE, sizeof(char));
	
	/*int maxChildren = 2;
	int curChildren = 0;
	pid_t* childProcs = (pid_t*)calloc(maxChildren, sizeof(pid_t));
	pid_t childID = 0;*/
	
	//Make the temporary file system
	char tempDir[] = "temp.XXXXXX\0";
	char hashFile[] = ".4220_file_list.txt\0";
	int hashFD = 0;
	SetupFiles(tempDir, hashFile, &hashFD);
	
	uint16_t childPort = 0;
	int childFD = MakeSocket(&childPort);
	
	
	int running = 1;
	
	while(running)
	{
		printf("Running!\n");	
		bzero(message, MAXSIZE);
		bzero(clientAddr, sizeof(struct sockaddr_in));

		amtRead = recvfrom(sockFD, message, MAXSIZE, 0, (struct sockaddr *) clientAddr, &addrLen);
		
		opCode = ParsePacket(message, result);
		printf("Opcode:  %d\n", opCode);
		
		/*If the request is for reading or writing a file, then make a child process to
		* Do so. */																		 
		if(opCode < 3)
		{
			
			//childID = fork();
				
			Child_Process (childFD, clientAddr, (struct RWPacket *) result);
			
			
			/*else if(childID > 0)
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
			}*/
		}
		/*Else, if the opcode tells us to do a contents request, then
		* send a write request for the file with the hashes. */
		else if (opCode == 6)
		{
			MakeContentRequest(hashFile, result);
			Child_Process(childFD, clientAddr, (struct RWPacket *) result);
			
			
		}										      
		else
		{
			SendErrorPacket(sockFD, 0, 
				"Cannot send requests other than read and write to the server.",
				clientAddr);
		}
		
		//CheckChildren(childProcs, &curChildren);
		
		
	}
	
	/***************************************
	* MAKE SURE TO DELETE THE TEMP DIR
	***************************************/
	
	
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
	
	
	
	printf("--------+++++++++ %d\n",opCode);
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

//Initialize File Structure
char* SetupFiles(char* tempDir, char* hashFile, int* hashFD)
{
	char* tempName = mkdtemp(tempDir);
	
	*hashFD = open(hashFile, O_CREAT|O_WRONLY|O_TRUNC);
	
	return tempName;
	
	
}

//Make A Content Write Request
void MakeContentRequest(char* hashFile, void* result)
{
	struct RWPacket* rw;
	rw = calloc(1, sizeof(char)*512);
	rw->OpCode = 2;
	//printf("%d\n", rw->OpCode);
	rw->Filename = strdup(hashFile);
	memcpy(result, rw,512);

}




