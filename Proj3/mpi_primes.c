#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
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
int SendVals(int* vals);


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
		Add_To_Int_Array(wheel->arr, wheel->wheel_size, wheel->entries);		
	} else if (wheel->entries == wheel->maxPrimes)
	{
		return;
	} else {
		Add_To_Int_Array(wheel->arr, wheel->wheel_size, wheel->entries);
	}
}

void Add_To_Int_Array(int* array, int arraySize, int entries)
{
	if(entries==arraySize-2)
	{
		
	}
}



















