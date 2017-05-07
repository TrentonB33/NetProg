/* This program attempts to recreate the MD5 hash, and does so almost perfectly.
*  The only problem is that when creating the integers inside the hashing loop,
*  The integers are not created correctly from the 4 chars that they represent
*/

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>


struct MD5Hash{
	char* filename;
	char hash[32];
};

int shifts[]=
{	7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
	5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
	4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
	6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21 };

unsigned int constants[]=
{	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
 	0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
 	0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
 	0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
 	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
 	0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
 	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
 	0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

//Function declarations
void MakeHash(char** hash, char* message, long msgSize);
unsigned int LeftRotate(unsigned int toShift, int shiftAmt);


//The function takes in a MD5Hash sturct with a filename, and then populates
//The hash field after running the contents of the file through the hash
void GetHash(struct MD5Hash* toPop)
{	
	int sizeInBits = 0;
	int targetSize = 0;
	int amtToZero = 0;
	
	struct stat stats;
	long sizeOfFile;
	long index = 0;
	
	char* result = (char*)calloc(32, sizeof(char));
	
	if(stat(toPop->filename, &stats) < 0)
	{
		printf("Stat Broke.\n");
		exit(EXIT_FAILURE);
	}
	
	sizeOfFile = (long)stats.st_size;
	index = sizeOfFile;
	
	//printf("Size of File: %ld\n", sizeOfFile);
	
	int fd = open(toPop->filename, O_RDONLY);
	
	char* buffer = (char*)calloc(sizeOfFile+1, sizeof(char));
	
	
	
	read(fd, buffer, sizeof(char)*sizeOfFile);
			
	buffer[sizeOfFile] = 0x80;
	//printf("first bit of the buffer: %x\n", buffer[sizeOfFile]);
	index++;
	
	//We need a buffer size that can fit into blocks of size 512 bits
	//After we've added the 1 bit.
	sizeInBits = (index)*8;
	targetSize = (sizeInBits+(512 - sizeInBits % 512))/8;
	amtToZero = (448 - sizeInBits % 512)/8;
	
	buffer = realloc(buffer, targetSize);
	bzero(&buffer[index], amtToZero+8);//I want to initialize the rest of the data
	
	index += amtToZero;
	//printf("Current Size of File: %ld\n", index);
	
	memcpy(&buffer[index], &sizeOfFile, sizeof(long));
	
	index += sizeof(long);	
	
	/*for(int i = 0; i < index; i += 4)
	{
		printf("%x\n", buffer[i]);
	}*/
	
	MakeHash(&result, buffer, index);
	memcpy(toPop->hash, result, 32);
	//printf("Hash on the outside: %s\n", toPop->hash);
	
}

void MakeHash(char** hash, char* message, long msgSize)
{
	unsigned int a0 = 0x67452301;
	unsigned int b0 = 0xefcdab89;
	unsigned int c0 = 0x98badcfe;
	unsigned int d0 = 0x10325476;
	
	unsigned int A1 = 0;
	unsigned int B1 = 0;
	unsigned int C1 = 0;
	unsigned int D1 = 0;
	unsigned int temp = 0;
	
	unsigned int funct = 0;
	int index = 0;
	
	int itr = 0;
	
	long start = 0;
	
	int* workspace = (int*)calloc(16, sizeof(int));
	
	int i = 0;
	while(start < msgSize)
	{
		for( i = 0; i < 16; i++)
		{
			memcpy(&workspace[i], &message[start + i*sizeof(int)], sizeof(int));
			//printf("Value: %x\n", workspace[i]);
		}
		
		A1 = a0;
		B1 = b0;
		C1 = c0;
		D1 = d0;
		
		for(itr = 0; itr < 64; itr++)
		{
			if(itr <= 15)
			{
				funct = (B1 & C1) | ((~B1) & D1);
				index = itr;				
				
			} else if(itr <= 31)
			{
				funct = (D1 & B1) | ((~D1) & C1);
				index = (5 * itr + 1) % 16;		
				
			} else if(itr <= 47)
			{
				funct = B1 ^ C1 ^ D1;
				index = (3 * itr + 5) % 16;
				
			} else
			{
				funct = C1 ^ (B1 | (~D1));
				index = (7 * itr) % 16;
				
			}
			
			temp = D1;
			D1 = C1;
			C1 = B1;
			B1 = B1 + LeftRotate(A1 + funct + constants[itr] + workspace[index], shifts[itr]);
			A1 = temp;
		}
		
		a0 += A1;
		b0 += B1;
		c0 += C1;
		d0 += D1;
		
		start += 64;
	}
	
	//printf("Hash: %x%x%x%x\n", a0, b0, c0, d0);
	//char buffer[32];
	sprintf(*hash, "%x%x%x%x", a0, b0, c0, d0);
	//write(1, *hash, 32);
	//printf("\n");
}

unsigned int LeftRotate(unsigned int toShift, int shiftAmt)
{
	return (toShift << shiftAmt) | (toShift >> (32 - shiftAmt));
}

int main()
{
	
	struct MD5Hash hash;
	hash.filename = "empty";
	
	GetHash(&hash);
	
		
	
	return EXIT_SUCCESS;
}

