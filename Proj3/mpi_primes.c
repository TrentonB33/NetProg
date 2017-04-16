#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <strings.h>
#include <string.h>

struct Wheel {
	int wheel_size;
	int entries;
	int maxPrimes;
	int* arr;	
};

//Function declarations
//Main Rank functions
int BroadcastWheel(struct Wheel* wheel);
int GetRankVals(int* vals, int numVals, int** toPopulate);
int GetValuesFromWoker(int rank, int** toPopulate);

//Worker Rank functions
int GetWheel(struct Wheel* toPopulate);
int SendVals(int* vals, int numVals);


//Array Managment
int Add_To_Int_Array(int** array, int arraySize, int entries, int item);
void Add_To_Wheel(struct Wheel* wheel, int* items, int numToAdd);
int* Wheel_Factorize(int _start, int _end, struct Wheel* wheel, int* count);

//Helper Functions
void PrintArray(int* array, int num);

//Testing Functions
void TrentonTesting();
int AddToWheelTest();
int GetSendValsTest();



int end_now = 0;
int worldSize, id, interEnd;

void sig_handler(int signo)
{
	//printf("DUh\n");
    if (signo == SIGUSR1) {
		//printf("WUh\n");
        end_now = 1;
    }
}

int main(int argc, char** argv)
{

	MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
	
    
    signal(SIGUSR1, sig_handler);
	
	int start = 10, end = 100, rankStart, rankEnd, found, stop = 0;
	int* output;
    int count = 0, primes = 4;
	struct Wheel* wheel = calloc(1, sizeof(struct Wheel));
	int* joinedPrimes;
	if(id == 0)
	{
		wheel->arr = calloc(10, sizeof(int));
		wheel->entries = 4;
		wheel->wheel_size = 10;
		wheel->arr[0] = 2;
		wheel->arr[1] = 3;
		wheel->arr[2] = 5;
		wheel->arr[3] = 7;
		//printf("%d\n", wheel->entries);
		BroadcastWheel(wheel);
		//output = Wheel_Factorize(3, 99, wheel, &count);
		//primes +
	} else {
		GetWheel(wheel);
	
	}
    
	
	
	
	
	
	//TrentonTesting();
	if(id == 0)
	{
		rankStart = start;
		printf("N\t\tPrimes\n");
		printf("10\t\t%d\n",primes);
	}
	
    while (1) {
		if(id == 0)
		{
			rankStart = start;
			rankEnd = (end/worldSize) * (id + 1);
		} else if(id == worldSize-1)
		{
			rankStart = (end/worldSize) * (id);
			rankEnd = end;
		} else {
			rankStart = (end/worldSize) * (id);
			rankEnd = (end/worldSize) * (id + 1);
		}
		count = 0;
		//printf("Stop: %d, Rank: %d, Start: %d, End: %d \n", stop, id, rankStart, rankEnd);
		
		
		//Communication! POOP People Order Our Primes!
		output = Wheel_Factorize(rankStart, rankEnd, wheel, &count);
		
		//PrintArray(output, count);
		if(id == 0)
		{
			found = GetRankVals(output, count, &joinedPrimes);
			primes += found;
			//PrintArray(joinedPrimes, found);
			Add_To_Wheel(wheel, joinedPrimes, found);
			//PrintArray(wheel->arr,wheel->entries);
			if(end_now==0)	printf("%d\t\t%d\n",end,primes);
			else printf("%d\t\t%d\n",interEnd,primes);
			BroadcastWheel(wheel);
			
		} else {
			SendVals(output, count);
			GetWheel(wheel);
		}
		
		if (end_now == 1 || stop == 30) {
			//printf()
			//printf("FINE!\n");
			MPI_Finalize();
            return 0;
        }
        
		stop++;
		start*=10;
		end*=10;
		
		
		
    }
	
	//printf("%d\n", primes);
	
	
    MPI_Finalize();
    
    return 0;
}


/**
	

**/

int* Wheel_Factorize(int _start, int _end, struct Wheel* wheel, int* count)
{
	int start = _start;
	int end = _end;
	int x = 0, arrsize = 10, y = 0, flag = 0;//, count = 0;
	int* output = calloc(arrsize, sizeof(int));
	if(start%2==0)
	{
		//printf("huh %d", start);
		start++;
	}
	if(end%2==0)
	{
		end--;
	}
	//printf("%d\n", wheel->entries);
	while(start<=end)
	{
		if (end_now == 1) {
			//printf("FINE!\n");
			interEnd = start;
            return output;
        }
		//printf("Rank %d: Num %d\n", id, start);
		//printf("\nduh  %d", start);
		for(y = 0; y < wheel->entries; y++)
		{
			if(start%wheel->arr[y] == 0)
			{
				//printf(" dumb \n");
				flag = -1;
				break;
			} else {
				//printf(" dooh   ");
				flag = 1;
			}
		}
		
		if(flag == 1)
		{
			//printf("output pointer value: %li\n", (long int) output);
			arrsize = Add_To_Int_Array(&output, arrsize, *count, start);\
			//printf("output pointer value (after): %li\n", (long int) output);
			//printf("%d\n",*count);
			for(x=0; x<*count;x++)
			{
				//printf("%d : ", output[x]);
			}
			//printf("\n");
			(*count)++;
		}
		
		start+=2;
	}
	
	for(x=0; x<*count;x++)
	{
		//printf("%d   %d\n", x, output[x]);
	}
	
	return output;
}

void Add_To_Wheel(struct Wheel* wheel, int* items, int numToAdd)
{
	int totalSize = wheel->entries + numToAdd;
	int* newList = (int*)calloc(totalSize, sizeof(int));
	
	memcpy(newList, wheel->arr, wheel->entries*sizeof(int));
	memcpy(newList + wheel->entries, items, numToAdd*sizeof(int));
	
	wheel->arr = newList;
	wheel->entries = totalSize;
	wheel->wheel_size = totalSize;
	
}

int Add_To_Int_Array(int** array, int arraySize, int entries, int item)
{
	if(entries>=arraySize-2)
	{
		//printf("Faggots ");
		arraySize = arraySize*2;
		//printf("Array before realloc: %li\n", (long int) array);
		*array = realloc(*array, sizeof(int) * arraySize);
		//printf("Array after realloc: %li\n", (long int) array);
		(*array)[entries] = item;
		//entries++;
	} else {
		(*array)[entries] = item;
		//entries++;
	}
	return arraySize;
}





/***********************************
*  COMMUNICATION FUNCTIONS		   *
***********************************/

//MAIN RANK FUNCTIONS

/********** BroadcastWheel **********
Params: struct Wheel wheel
Returns: 0 if successful, or an error code otherwise

BroadcastWheel takes a wheel to broadcast and sends that wheel to all the
Worker Ranks. Nothing else special is done.
*/

int BroadcastWheel(struct Wheel* wheel)
{
	/*int MPI_Ibcast(void *buffer, int count, MPI_Datatype datatype,
    int root, MPI_Comm comm, MPI_Request *request)*/
	
	MPI_Request res1, res2;
	MPI_Status stat1, stat2;
	
	int numIntInWheel = 3;
	
	int size = wheel->entries + numIntInWheel;
	
	int buffer[size];
	buffer[0] = wheel->wheel_size;
	buffer[1] = wheel->entries;
	buffer[2] = wheel->maxPrimes;
	
	memcpy(buffer+numIntInWheel, wheel->arr, sizeof(int)*wheel->entries);
	
	int itr;
	
	for(itr = 1; itr < worldSize; itr++)
	{
		MPI_Isend(&size, 1, MPI_UNSIGNED, itr, 0, MPI_COMM_WORLD, &res1);
	
		MPI_Isend(buffer, size, MPI_UNSIGNED,itr, 0, MPI_COMM_WORLD, &res2);
		
		MPI_Wait(&res1, &stat1);
		MPI_Wait(&res2, &stat2);
	}
	/*MPI_Ibcast(&size, 1, MPI_UNSIGNED,
    		0, MPI_COMM_WORLD, &res1);
	
	MPI_Ibcast(buffer, size, MPI_UNSIGNED,
    		0, MPI_COMM_WORLD, &res2);*/
	
	//PrintArray(buffer, size);
	
	return 0;
}


/********** GetRankVals ***********
Params: int* vals, int numVals, int** toPopulate
Returns: The number of elements in toPopulate

GetRankVals waits for worker ranks to send their possible prime number to the main rank
and then combines these values.  It combines these values with the values passed in with vals
It then puts the combined array into toPopulate and returns the number of elements 
that have been put into toPopulate. Order from least to greatest is assured in toPopulate.
*/
int GetRankVals(int* vals, int numVals, int** toPopulate)
{
	
	int itr;
	int totalMembers = numVals;
	int** partials = (int**) calloc(worldSize, sizeof(int*));
	int counts[worldSize];
	counts[0] = numVals;

	for(itr = 1; itr < worldSize; itr++)
	{
		partials[itr] = (int*)calloc(1, sizeof(int));
		counts[itr] = GetValuesFromWoker(itr, &partials[itr]);
		totalMembers += counts[itr];
	}
	
	*toPopulate = (int*) calloc(totalMembers, sizeof(int));

	memcpy(*toPopulate, vals, numVals*sizeof(int));
	
	int offset = 0;
	
	for(itr = 1; itr < worldSize; itr++)
	{
		offset += counts[itr-1];
		memcpy(*toPopulate + offset, partials[itr], counts[itr]*sizeof(int));
		
	}
	
	return totalMembers;
}


int GetValuesFromWoker(int rank, int** toPopulate)
{
	MPI_Request res1, res2;
	MPI_Status stat1, stat2;
	
	int size = 0;
	
	MPI_Irecv(&size, 1, MPI_UNSIGNED, rank, MPI_ANY_TAG,
    		MPI_COMM_WORLD, &res1);
	
	MPI_Wait(&res1, &stat1);
	
	*toPopulate = (int*) calloc(size, sizeof(int));
	
	MPI_Irecv(*toPopulate, size, MPI_UNSIGNED, rank, MPI_ANY_TAG,
    		MPI_COMM_WORLD, &res2);
	
	MPI_Wait(&res2, &stat2);
	
	//PrintArray(*toPopulate, size);
	
	return size;
	
}

//WORKER RANK FUNCTIONS

/********** GetWheel **********
Params: struct Wheel toPopulate
Returns: 0 if successful, or an error code otherwise

GetWheel expects a wheel struct, which it then populates with the wheel data
That is received from the main rank
*/
int GetWheel(struct Wheel* toPopulate)
{
	MPI_Request res1, res2;
	MPI_Status stat1, stat2;
	
	int size = 0;
	int numIntInWheel = 3;
	
	MPI_Irecv(&size, 1, MPI_UNSIGNED, 0, MPI_ANY_TAG,
    		MPI_COMM_WORLD, &res1);
	
	MPI_Wait(&res1, &stat1);
	
	int buffer[size];
	
	MPI_Irecv(buffer, size, MPI_UNSIGNED, 0, MPI_ANY_TAG,
    		MPI_COMM_WORLD, &res2);
	
	MPI_Wait(&res2, &stat2);
	
	/*MPI_Ibcast(&size, 1, MPI_UNSIGNED,
    		0, MPI_COMM_WORLD, &res1);
	
	MPI_Wait(&res1, &stat1);
	
	printf("Worker made it past the INIT!\n");
	scanf("\n");
	
	int buffer[size];
	
	MPI_Ibcast(buffer, size, MPI_UNSIGNED,
    		0, MPI_COMM_WORLD, &res2);
	
	MPI_Wait(&res2, &stat2);*/
	
	toPopulate->wheel_size = buffer[0];
	toPopulate->entries = buffer[1];
	toPopulate->maxPrimes = buffer[2];
	
	toPopulate->arr = (int*) calloc(toPopulate->entries, sizeof(int));
	memcpy(toPopulate->arr, buffer+numIntInWheel, toPopulate->entries*sizeof(int));
	
	//PrintArray(buffer, size);
	
	
	return 0;
}

/********** SendVals **********
Params: int* vals, int numVals
Returns: 0 if successful, or an error code otherwise

SendVals expects an array of integers ordered from least to greatest, and the number of
elements in that array.  it send this array to the main rank.
*/
int SendVals(int* vals, int numVals)
{
	MPI_Request res1, res2;
	//MPI_Status stat1, stat2;
	
	//printf("Rank %d: Entering the child sending Proc.\n", id);
	
	
	MPI_Isend(&numVals, 1, MPI_UNSIGNED, 0, 0,
    		MPI_COMM_WORLD, &res1);
	
	//MPI_Wait(&res1, &stat1);
	
	MPI_Isend(vals, numVals, MPI_UNSIGNED, 0, 0,
    		MPI_COMM_WORLD, &res2);
	
	//MPI_Wait(&res2, &stat2);
	
	//printf("Rank %d: Exiting the child sending Proc.\n", id);
	
	return 0;
}


//Helper Functions:
void PrintArray(int* array, int num)
{
	int i;
	printf("Rank %d: ", id);
	for(i = 0; i < num; i++)
	{
		printf("%d ", array[i]);
	}
	printf("\n");
}

//Testing Functions
void TrentonTesting()
{
	
	if(AddToWheelTest() != 0)
	{
		printf("Add to wheel test failed.\n");
	} else if(GetSendValsTest() != 0)
	{
		printf("Get/Send Vals test failed.\n");
	} 
	
}

int AddToWheelTest()
{
	struct Wheel* wheel = calloc(1, sizeof(struct Wheel));
	wheel->arr = calloc(10, sizeof(int));
	wheel->entries = 4;
	wheel->wheel_size = 10;
	wheel->arr[0] = 2;
	wheel->arr[1] = 3;
	wheel->arr[2] = 5;
	wheel->arr[3] = 7;
	
	int buffer[10];
	int itr;
	
	for(itr = 0; itr < 10; itr++)
	{
		buffer[itr] = itr*10;
	}
	
	Add_To_Wheel(wheel, buffer, 10);
	
	PrintArray(wheel->arr, wheel->entries);
	
	if(wheel->entries == 14)
	{
		return 0;
	}
	else return 1;
}

int GetSendValsTest()
{
	int size = (id + 1) * 5;
	int buffer[size];
	int itr;
	
	for(itr = 0; itr < size; itr++)
	{
		buffer[itr] = itr;
	}
	
	if(id == 0)
	{
		int* allVals = (int*)calloc(1,sizeof(int));
		int count = GetRankVals(buffer, size, &allVals);
		PrintArray(allVals, count);
	}
	else
	{
		SendVals(buffer, size);
	}
	
	return 0;
}
















