#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>

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


int end_now = 0;

void sig_handler(int signo)
{
    if (signo == SIGUSR1) {
        end_now = 1;
    }
}

int main(int argc, char** argv)
{
    int count, id;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &count);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    
    signal(SIGUSR1, sig_handler);
    
    while (1) {
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
	return 0;
}

/********** GetRankVals ***********
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

/********** GetWheel ***********
Params: struct Wheel toPopulate
Returns: 0 if successful, or an error code otherwise

GetWheel expects a wheel struct, which it then populates with the wheel data
That is received from the main rank
*/
int GetWheel(struct Wheel* toPopulate)
{
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
	return 0;
}

















