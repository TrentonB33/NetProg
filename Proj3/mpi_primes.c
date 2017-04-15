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



















