#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include  "p3150028-p3140236-mysh-common.c"

#define MAX_ENTOLI 255

int main() {
	char* entoli   = (char*)malloc(MAX_ENTOLI * sizeof(char));				//allocates the requested memory and returns a pointer to it.
	sprintf(entoli, "quit");												//is used to store formatted data as  a string
	shellN(2);																//printf("mysh2>");
	gets(entoli);															// reads  a  line until  a terminating newline or EOF
	while (strcmp(entoli, "quit") != 0) {									//as long as entoli is NOT Ctrl+D
		pid_t pid;
		pid = fork();														//the creation of a child process
		if (pid > 0)														// I am the parent process
		 {
			pid_t waitPid,status;
			waitPid = wait(&status);										//and waiting for my child process to finish
			if (waitPid == -1) 												//Failed wait
			{
				printf("ERROR, wait failed!\n");
				exit(-1);
			}
		} else if (pid == 0) {												//I am the child process										
			char delimiter[1]; 												//an array of delimiters for strtok
			delimiter[0] = ' '; 											//one delimiter,blank space

			char** 	 argv = (char**)malloc(sizeof(char*));				//an array for string( string is an array of characters),
																		//allocate memory for the the  array of characters	
			int args = 0; 												//how many arguments(orismata) has entoli

			char* 	token = strtok(entoli, delimiter);			//each time has one argument( orisma) from  entoli,take the next argument(orisma)
													
			while (token != NULL)													//as long as there are other tokens
			 { 													
				argv[args] = (char*)malloc(strlen(token) * sizeof(char)); 		//allocate  memory  for the argument( orisma)
				sprintf(argv[args],"%s",token);									//is used to store formatted data as  a string
				args++;															//increase the arguments +1
				token = strtok(NULL, delimiter);		//In each subsequent call that should  parse the same string,str must be NULL				
			}

			argv[args] = NULL;										//Indicates the end of arguments.
			int execution;
			execution = execvp(argv[0], argv);						//argv[0] has the entoli name 
																	//argv is a pointer to an array of pointers to null-terminated character  
																	//strings. A  NULL pointer is used to mark the end of the array.
			
			if (execution == -1) 												//Failed execvp
			{						
				printf("ERROR, execlp failed at command %s!\n", entoli);
			}
			exit(0);															//child process finished

		} else if (pid == -1) {													//Failed fork
			printf("ERROR, fork failed\n");
		}

		shellN(2);																//printf("mysh2>");
		sprintf(entoli, "quit");												//is used to store formatted data as  a string
		gets(entoli);															// reads  a  line until  a terminating newline or EOF
	}
	free(entoli);																// deallocates the memory previously allocated by malloc
	printf("\n");
	return 0;
}
