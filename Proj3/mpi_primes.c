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
int* GetRankVals();

//Worker Rank functions
int GetWheel(struct Wheel* toPopulate);
int SendVals(int* vals, int numVals);


//Array Managment
int Add_To_Int_Array(int** array, int arraySize, int entries, int item);
void Add_To_Wheel(struct Wheel* wheel, int item);
int* Wheel_Factorize(int _start, int _end, struct Wheel* wheel, int* count);

//Helper Functions
void PrintArray(int* array, int num);



int end_now = 0;
int worldSize, id;

void sig_handler(int signo)
{
    if (signo == SIGUSR1) {
        end_now = 1;
    }
}

int main(int argc, char** argv)
{

	MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
	
    
    signal(SIGUSR1, sig_handler);
	
	int* output, x;
    int count = 4;
	struct Wheel* wheel = calloc(1, sizeof(struct Wheel));
	if(id == 0)
	{
		wheel->arr = calloc(10, sizeof(int));
		wheel->entries = 4;
		wheel->wheel_size = 10;
		wheel->arr[0] = 2;
		wheel->arr[1] = 3;
		wheel->arr[2] = 5;
		wheel->arr[3] = 7;
		printf("%d\n", wheel->entries);
		BroadcastWheel(wheel);
		output = Wheel_Factorize(3, 99, wheel, &count);
		
	} else {
		GetWheel(wheel);
		for(x=0; x<wheel->entries;x++)
	{
		printf("%d   %d\n", x, wheel->arr[x]);
	}
	}
    
	
	
	
	printf("%d\n", count);
	

    /*while (1) {
		end_now = 1;
        if (end_now == 1) {
            break;
        }
    }*/
	
	
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
		start--;
	}
	if(end%2==0)
	{
		end++;
	}
	//printf("%d\n", wheel->entries);
	while(start<=end)
	{
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

void Add_To_Wheel(struct Wheel* wheel, int item)
{
	if(wheel->maxPrimes == 0)
	{
		wheel->wheel_size = Add_To_Int_Array(&wheel->arr, wheel->wheel_size, wheel->entries, item);
		wheel->entries++;
	} else if (wheel->entries >= wheel->maxPrimes)
	{
		return;
	} else {
		wheel->wheel_size = Add_To_Int_Array(&wheel->arr, wheel->wheel_size, wheel->entries, item);
		wheel->entries++;
	}
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
	
	MPI_Ibcast(&size, 1, MPI_UNSIGNED,
    		0, MPI_COMM_WORLD, &res1);
	
	MPI_Ibcast(buffer, size, MPI_UNSIGNED,
    		0, MPI_COMM_WORLD, &res2);
	
	MPI_Wait(&res1, &stat1);
	MPI_Wait(&res2, &stat2);
	
	PrintArray(buffer, size);
	
	return 0;
}

/********** GetRankVals **********
Params: none
Returns: An array of integer values that have been received from the worker ranks

GetRankVals waits for worker ranks to send their possible prime number to the main rank
and then combines these values.  It then returns this complete array to the caller.
Order from least to greatest is assured.
*/
int* GetRankVals()
{
	return 0;
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

/********** SendVals **********
Params: int* vals, int numVals
Returns: 0 if successful, or an error code otherwise

SendVals expects an array of integers ordered from least to greatest, and the number of
elements in that array.  it send this array to the main rank.
*/
int SendVals(int* vals, int numVals)
{
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

















