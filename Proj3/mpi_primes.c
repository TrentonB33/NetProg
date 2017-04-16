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

//Helper Functions
void PrintArray(int* array, int num);


int end_now = 0;
int count, id;

void sig_handler(int signo)
{
    if (signo == SIGUSR1) {
        end_now = 1;
    }
}

int main(int argc, char** argv)
{
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &count);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
	
    
    signal(SIGUSR1, sig_handler);
	
	int num = (id+1)*5;
	
	int myVals[num];
	int itr;

	for(itr = 0; itr < num; itr++)
	{
		myVals[itr] = itr;
	}
	
	if(id == 0)
	{
		int* vals = (int*)calloc(1, sizeof(int));
		int count = GetRankVals(myVals, num, &vals);
		
		PrintArray(vals, count);
	} else
	{
		
		SendVals(myVals, num);
		
	}
    
    while (1) {
		end_now = 1;
        if (end_now == 1) {
            break;
        }
    }
    
    MPI_Finalize();
    
    return 0;
}


/**
	

**/

int* Wheel_Factorize(int _start, int _end, struct Wheel* wheel)
{
	int start = _start;
	int end = _end;
	int x = 0, arrsize = 10, entries = 0;
	if(start%2==0)
	{
		start--;
	}
	if(end%2==0)
	{
		end++;
	}
	while(x<=end)
	{
		for(x=0; x<wheel->entries;x++)
		{
			
		}
	}
	return NULL;
}

void Add_To_Wheel(struct Wheel* wheel, int item)
{
	if(wheel->maxPrimes == 0)
	{
		Add_To_Int_Array(wheel->arr, &wheel->wheel_size, &wheel->entries, item);
	} else if (wheel->entries == wheel->maxPrimes)
	{
		return;
	} else {
		Add_To_Int_Array(wheel->arr, &wheel->wheel_size, &wheel->entries, item);
	}
}

void Add_To_Int_Array(int* array, int* arraySize, int* entries, int item)
{
	if(*entries==*arraySize-2)
	{
		array = realloc(array, sizeof(int)*(*arraySize) * 2);
		*arraySize = *arraySize*2;
		array[*entries+1] = item;
		*entries++;
	} else {
		array[*entries+1] = item;
		*entries++;
	}
}





/***********************************
*  COMMUNICATION FUNCTIONS
***********************************/

//MAIN RANK FUNCTIONS

/********** BroadcastWheel ***********
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
	
	MPI_Ibcast(&size, 1, MPI_UNSIGNED,
    		0, MPI_COMM_WORLD, &res1);
	
	MPI_Ibcast(buffer, size, MPI_UNSIGNED,
    		0, MPI_COMM_WORLD, &res2);
	
	MPI_Wait(&res1, &stat1);
	MPI_Wait(&res2, &stat2);
	
	PrintArray(buffer, size);
	
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
	int** partials = (int**) calloc(count, sizeof(int*));
	int counts[count];
	counts[0] = numVals;
		
	//printf("Rank %d: Entering the receiving Proc.\n", id);
	
	//PrintArray(vals,numVals);
	for(itr = 1; itr < count; itr++)
	{
		partials[itr] = (int*)calloc(1, sizeof(int));
		counts[itr] = GetValuesFromWoker(itr, &partials[itr]);
		totalMembers += counts[itr];
		//PrintArray(partials[itr], counts[itr]);
	}
	
	/*printf("Rank %d: Should have all the vals.\n", id);
	PrintArray(counts, count);
	printf("Rank %d: totalMembers %d.\n", id, totalMembers);*/
	
	
	*toPopulate = (int*) calloc(totalMembers, sizeof(int));
	//PrintArray(vals,numVals);
	memcpy(*toPopulate, vals, numVals*sizeof(int));
	
	//PrintArray(*toPopulate, numVals);
	
	//printf("Rank %d: Have the first vals Count %d.\n", id, count);
	
	int offset = 0;
	
	for(itr = 1; itr < count; itr++)
	{
		offset += counts[itr-1];
		memcpy(*toPopulate + offset, partials[itr], counts[itr]*sizeof(int));
		
	}
	
	//printf("Rank %d: Exiting the receiving Proc.\n", id);
	
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

/********** GetWheel ***********
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
	
	MPI_Ibcast(&size, 1, MPI_UNSIGNED,
    		0, MPI_COMM_WORLD, &res1);
	
	MPI_Wait(&res1, &stat1);
	
	/*printf("Worker made it past the INIT!\n");
	scanf("\n");*/
	
	int buffer[size];
	
	MPI_Ibcast(buffer, size, MPI_UNSIGNED,
    		0, MPI_COMM_WORLD, &res2);
	
	MPI_Wait(&res2, &stat2);
	
	toPopulate->wheel_size = buffer[0];
	toPopulate->entries = buffer[1];
	toPopulate->maxPrimes = buffer[2];
	
	toPopulate->arr = (int*) calloc(toPopulate->entries, sizeof(int));
	memcpy(toPopulate->arr, buffer+numIntInWheel*sizeof(int), toPopulate->entries*sizeof(int));
	
	PrintArray(buffer, size);
	
	return 0;
}

/********** SendVals ***********
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

















