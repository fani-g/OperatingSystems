#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>


// prints the cells (upopinakes),where from is the start of upopinakas and to is the end of upopinakas
 
void print_array(char* title, int* array, int from, int to) {
	int cell;
	printf("%s:\n", title);
	for (cell = from; cell < to - 1; cell++) {
		printf("%d\n", array[cell]);
	}
	printf("%d\n\n", array[to - 1]);

}


