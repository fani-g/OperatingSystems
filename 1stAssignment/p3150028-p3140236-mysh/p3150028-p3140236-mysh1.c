#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include "p3150028-p3140236-mysh-common.c"

#define MAX_ENTOLI 255

int main() {
	char entoli[MAX_ENTOLI];							//an array of characters of the user's command
	sprintf(entoli, "quit");							//is used to store formatted data as  a string
	shellN(1);											//printf("mysh1>");
	scanf("%s",entoli);									//read entoli from keyboard
	while (strcmp(entoli, "quit") != 0) {				//as long as entoli is NOT Ctrl+D
		pid_t pid;
		pid = fork();      								// the creation of a child process
		if (pid > 0)						 			//I am the parent process
		 {    								
			pid_t  waitPid,status;
			waitPid = wait(&status);            		//and  waiting for my child process to finish
			if (waitPid == -1)							//Failed wait
			 {
				printf("ERROR, wait failed!\n");
				exit(-1);
			}
		} else if (pid == 0) { 							//I am the child process
			int execution;
			execution = execlp(entoli, entoli, NULL);   // e.g  execlp("ls","ls",NULL)
			if (execution == -1) 						//Failed execlp
			{
				printf("ERROR, execlp failed!\n");
			}
			exit(0);									//child process finished

		} else if (pid == -1) {							// Failed fork
			printf("ERROR, fork failed\n");
		}

		shellN(1);										//printf("mysh1>");
		sprintf(entoli, "quit");						//is used to store formatted data as  a string
		scanf("%s",entoli);								//read entoli from keyboard
	}

	return 0;
}
