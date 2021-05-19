#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "p3150028-p3140236-os_ex2.h"


int* array;           					   //the array that has the numbers that we want to sort
int printer_id;       					   //which thread prints
pthread_mutex_t screen_mutex;			   //mutex prints each time 1 thread to the screen
pthread_mutex_t array_mutex;               //mutex gia apokleistikh prosbash to the array depends on mode
pthread_cond_t* cond;                      //condition variables ,in which order the thread will print

 
void read_arguments(int* numbers, int* threads, int* seed, int* mode, int argc, char** argv) {
	int argument;
	for (argument = 1; argument < argc; argument++)                   //reading the arguments
	{
		if (strncmp(argv[argument], "-numbers=", 9) == 0) 
		{
			*numbers = atoi(argv[argument] + 9);
		} else if (strncmp(argv[argument], "-threads=", 9) == 0){
			*threads = atoi(argv[argument] + 9);
			if (*threads != 1 && *threads != 2 && *threads != 4 && *threads != 8) 
			{
				printf("invalid number of threads set: %d\n", *threads);    // wrong number of threads 
				exit(0);
			}
		} else if (strncmp(argv[argument], "-seed=", 6) == 0) {
			*seed = atoi(argv[argument] + 6);
		} else if (strncmp(argv[argument], "-mode=", 6) == 0) {
			*mode = atoi(argv[argument] + 6);
			if (*mode != 1 && *mode != 2 && *mode != 3) 
			{
				printf("Invalid number of mode set: %d\n", *mode);          //wrong number of mode 
				exit(0);
			}
		} else {
			printf("Invalid arguments set, found: %s\n", argv[argument]);  //wrong argument
			exit(0);
		}
	}
}


void create_array(int array_size, int seed) {
	srand(seed);										//puts the seed
	array = (int*) malloc(array_size * sizeof(int));    // creation of the array
	if (array == NULL) {
		printf("Error during malloc for array\n");		//malloc failed
		exit(0);
	}
	int cell;
	for (cell = 0; cell < array_size; cell++) {
		array[cell] = rand() % (array_size * 4);        // fills the array with the random numbers
	}
}


//partition of quicksort
int partition(int p, int r, int mode) {
	int i = p - 1;
	
	if (mode == 1) {									//the absolute way , only one thread reads and writes
		if (pthread_mutex_lock(&array_mutex) != 0)
		{
			printf("Error locking mutex\n");			//if pthread_mutex_lock(&array_mutex) == 0 means that
			exit(0);									//the thread went to the critical area to read and write
		}
		int x = array[r];
		int j;
		for (j = p; j <= r - 1; j++) {
			if (array[j] <= x) {
				i = i + 1;
				int temp = array[i];
				array[i] = array[j];
				array[j] = temp;
			}
		}
		int temp = array[i + 1];
		array[i + 1] = array[r];
		array[r] = temp;
		
		if (pthread_mutex_unlock(&array_mutex) != 0)     //if pthread_mutex_unlock(&array_mutex) == 0 means that 
		{												// the thread unlock in order to go the next thread
			printf("Error unlocking mutex\n");
			exit(0);
		}
		
	} else if (mode == 2) {                        // the 'relaxed' way , many threads reads but only one thread writes ,locks for writing
		int x = array[r];
		int j;
		for (j = p; j <= r - 1; j++) 
		{
			if (array[j] <= x)
			 {
				i = i + 1;
				int temp = array[i];
				if (pthread_mutex_lock(&array_mutex) != 0)   
				 {
					printf("Error locking mutex\n");
					exit(0);
				}
				array[i] = array[j];
				array[j] = temp;
				if (pthread_mutex_unlock(&array_mutex) != 0) 
				{
					printf("Error unlocking mutex\n");
					exit(0);
				}
			}
		}
		int temp = array[i + 1];
		if (pthread_mutex_lock(&array_mutex) != 0) 				
		{
			printf("Error locking mutex\n");
			exit(0);
		}
		array[i + 1] = array[r];
		array[r] = temp;
		if (pthread_mutex_unlock(&array_mutex) != 0)           
		{
			printf("Error unlocking mutex\n");
			exit(0);
		}
		
	} else {									//Asynchronously.All threads are able to read and write and increases the performance
		int x = array[r];
		int j;
		for (j = p; j <= r - 1; j++) {
			if (array[j] <= x) {
				i = i + 1;
				int temp = array[i];
				array[i] = array[j];
				array[j] = temp;
			}
		}
		int temp = array[i + 1];
		array[i + 1] = array[r];
		array[r] = temp;
	}
	return i + 1;
}


 
void quicksort(int p, int r, int mode) {
	if (p < r) {							   //p the start of the array , r the end of the array
		int q = partition(p, r, mode);         //quicksort sorts an array from p to r
		quicksort(p, q - 1, mode);
		quicksort(q + 1, r, mode);
	}
}


 // the function that every thread execute. The arguments that we want to pass to the threads (threadId,which upopinaka to sort and mode)

void* sort(void* arguments) {
	int* thread_arguments = (int*) arguments;
	int thread_id = thread_arguments[0];
	int from = thread_arguments[1];
	int to = thread_arguments[2];
	int mode = thread_arguments[3];

	if (pthread_mutex_lock(&screen_mutex) != 0) {
		printf("Error locking mutex\n");
		exit(0);
	}
	printf("Started thread %d for sorting cells from %d to %d\n", thread_id, from, to);
	if (pthread_mutex_unlock(&screen_mutex) != 0) {
		printf("Error unlocking mutex\n");
		exit(0);
	}
	quicksort(from, to, mode);
	if (pthread_mutex_lock(&screen_mutex) != 0) {
		printf("Error locking mutex\n");
		exit(0);
	}

																			//wait to come your turn in order to print
	while (printer_id != thread_id) {
		if (pthread_cond_wait(&cond[thread_id], &screen_mutex) != 0) {
			printf("Error cond_wait\n");
			exit(0);
		}
	}

	
	char title[128];												//print upopinaka
	sprintf(title,"Thread %d finished", thread_id);
	print_array(title, array, from, to);

	printer_id--;
	if (pthread_cond_signal(&cond[printer_id]) != 0) {				//Descending order
		printf("Error cond signal\n");
		exit(0);
	}
	if (pthread_mutex_unlock(&screen_mutex) != 0) {
		printf("Error unlocking mutex\n");
		exit(0);
	}
	free(arguments);												//free memory
	return NULL;
}


 
int get_next(int* start) {					  //pulls under the arrow start down in the process of merge
	*start = * start + 1;
	return array[*start - 1];
}
 
int* find_next(int start, int end) {		  //takes the value from tha arrow start in the process of merge
	if (start <= end) {
		return &array[start];
	} else {
		return NULL;
	}
}


 //merge upopinakes of threads into the merged_array sub_arrays_num and each has size sub_array_size

void merge(int* merged_array, int sub_arrays_num, int sub_array_size) {
													
	int start[sub_arrays_num];								//arrows start upopinakwn
	int end[sub_arrays_num];								//arrowsvend upopinakwn							
	int i;
	for (i = 0; i < sub_arrays_num; i++) { 					//find the initial start and end of our array 
		start[i] = i * sub_array_size;
		end[i] = (i+1) * sub_array_size - 1;
	}

	int cell;
	int n = sub_arrays_num * sub_array_size;					//size of merged_array
	for (cell = 0; cell < n; cell++) {
		int min;
		int min_i = -1; 										//the position of the min
		int min_set = 0;										//find the min
		for (i = 0; i < sub_arrays_num; i++) {
			int* element = find_next(start[i], end[i]);
			if (element != NULL) {
				if (min_set == 0) { 							//the first element, don't compare with the min
					min_set = 1;
					min_i = i;
					min = *element;
				} else {
					if (*element < min) {
						min_i = i;
						min =  *element;
					}
				}
			}
		}
		merged_array[cell] = get_next(&start[min_i]);
	}
}


 // writes an array with titlo tilte and size array_size into a file
 
void write_array(int* array, int array_size, char* title, FILE* file) {
																					//writes the title 
	int items = fwrite(title, strlen(title), 1, file);							 
	if (items != 1) {
		printf("Error while writing to results.dat the title: %s\n", title);
		exit(0);
	}
																					//writes  ={
	char left[3] = "={";
	items = fwrite(left, strlen(left) , 1, file);
	if (items != 1) {
		printf("Error while writing to results.dat the ={ : \n");
		exit(0);
	}
	char number[32];
	int cell;
	for (cell = 0; cell < array_size - 1; cell++) {
																					//writes numbers
		sprintf(number, "%d", array[cell]);
		items = fwrite(number, strlen(number), 1, file);
		if (items != 1) {
			printf("Error while writing to results.dat the %s\n", number);
			exit(0);
		}

		
		char comma[3] = ", ";															//write , between numbers
		items = fwrite(comma, strlen(comma), 1, file);
		if (items != 1) {
			printf("Error while writing to results.dat the ,\n");
			exit(0);
		}
	}
																		//writes the last number, we don't put komma after the last number
	sprintf(number, "%d", array[array_size - 1]);
	items = fwrite(number, strlen(number), 1, file);
	if (items != 1) {
		printf("Error while writing to results.dat the %s\n", number);
		exit(0);
	}
																						//writes }
	char right[3] = "}\n";
	items = fwrite(right, strlen(right), 1, file);
	if (items != 1) {
		printf("Error while writing to results.dat the } : \n");
		exit(0);
	}
}

int main(int argc, char** argv) {							
	if (argc != 5) {
		printf("Wrong number of arguments!\n");       
		exit(0);
	}
	int n;   //numbers
	int t;   //thread
	int seed; 
	int mode;
	read_arguments(&n, &t, &seed, &mode, argc, argv);          //reads the arguments numbers,threads,seed mode

	struct timeval start;										//get date and time when the program starts
	if (gettimeofday(&start, NULL) == -1) 
	{
		printf("Error in gettimeofday\n");
		exit(0);
	}

	printf("Sorting %d numbers with %d threads with %d seed and %d mode:\n", n, t, seed, mode);
	create_array(n, seed);

	FILE* file = fopen("results.dat","w");									//open the file with the name results.dat

	if (file == NULL) 
	{
		printf("Error opening file results.dat\n");							//open failed
		exit(0);
	}

	write_array(array, n, "Initial Array", file);							//write to the file results.dat the initial array

	pthread_t threads[t];				
	cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t) * t);			//array of threads(1,2,4,8)
	if (cond == NULL) {
		printf("Error during malloc for cond\n");
		exit(0);
	}
	printer_id = t;
	int thread_id;
	for (thread_id = 1; thread_id <= t; thread_id++) {
		int* thread_arguments = (int*) malloc(sizeof(int) * 4);
		if (thread_arguments == NULL) {
			printf("Error during malloc for thread arguments\n");
		}
		thread_arguments[0] = thread_id;									// thread id
		thread_arguments[1] = (thread_id - 1) * (n / t);					// numbers/ threads 
		thread_arguments[2] = thread_id * (n / t) - 1;						// e.g 10 numbers and 2 threads ,5 numbers each
		thread_arguments[3] = mode;											//mode
		if (pthread_create(&threads[thread_id - 1], NULL, sort, (void*)thread_arguments) != 0) {
			printf("Error creating thread %d\n", thread_id);
			exit(0);
		}
	}
	for (thread_id = 1; thread_id <= t; thread_id++) {
		if (pthread_join(threads[thread_id - 1], NULL) != 0) {
			printf("Error joining thread %d\n", thread_id);
			exit(0);
		}
	}
	int merged_array[n];
	merge(merged_array, t, n / t);
	write_array(merged_array, n, "Sorted Array", file);      // writes the sorted array into the file

	if (fclose(file) != 0) 
	{
		printf("Error closing file results.dat\n");			//close failed
		exit(0);
	}

	free(array);											//free the memory
	free(cond);

	struct timeval end;										//get date and time when the program finishes
	if (gettimeofday(&end, NULL) == -1) 
	{
		printf("Error in gettimeofday\n");
		exit(0);
	}

	printf("Mergesort completed after %ld microseconds\n", end.tv_usec - start.tv_usec);
	return 0;
}




