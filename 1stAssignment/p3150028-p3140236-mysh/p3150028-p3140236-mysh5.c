#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>									 
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include  "p3150028-p3140236-mysh-common.c"

#define MAX_ENTOLI 255
#define MAX_PIPES 100



int main() {
	char* entoli = (char*)malloc(MAX_ENTOLI * sizeof(char));							//allocates the requested memory and returns a pointer to it.
	sprintf(entoli, "quit");															//is used to store formatted data as  a string
	shellN(5);																			//printf("mysh5>");
	gets(entoli);																		// reads  a  line until  a terminating newline or EOF
	while (strcmp(entoli, "quit") != 0) {												//as long as entoli is NOT Ctrl+D
		pid_t pid;
		pid = fork();																	//the creation of a child process
		if (pid > 0) {																	// I am the parent process
			pid_t  waitPid, status;
			waitPid = wait(&status);													//and waiting for my child process to finish
			if (waitPid == -1) {														//Failed wait
				printf("ERROR, wait failed!\n");
				exit(-1);
			}
			shellN(5);																	//printf("mysh5>");
			sprintf(entoli, "quit");													//is used to store formatted data as  a string
			gets(entoli);																// reads  a  line until  a terminating newline or EOF
		} else if (pid == 0) {															//I am the child process
			char delimiter[1]; 															//an array of delimiters for strtok
			delimiter[0] = ' '; 														//one delimiter,blank space

			char** 	argv = (char**)malloc(sizeof(char*));					//an array for string( string is an array of characters) 																				//,allocate memory for the the array of characters
			int args = 0;													//how many arguments(orismata) has entoli

			char* token;														//each time has one argument from entoli
			token = strtok(entoli, delimiter);									//take the next argument
			int swlhnwsh = -1;
			int pipefds[MAX_PIPES][2];											//two dimension array with pipes
			int cpid;		
			while (token != NULL) { 											//as long as there are other arguments
				if (token[0] == '<' && token[1]=='\0')							//redirect input
				 { 												
					token = strtok(NULL, delimiter);							//the next  token from  < has the name of the file input
					if (token == NULL)
					 {						
						printf("ERROR, file input name expected\n");
					} else {
						int fd = open(token, O_RDONLY); 					//Open the file input, reading only
						if (fd == -1) 										//Failed open
						{
							printf("ERROR, open failed\n");
						} else {
							if (dup2(fd, 0) == -1) 							//redirect the keyboard to the file
							{ 										
								printf("ERROR, dup2 failed\n");
							}
						}
					}
				}else if(token[0] == '>' && token[1] == '>' && token[2]=='\0' ){ 			//redirect output
					token = strtok(NULL, delimiter);								//the next  token from  >> has the name of the file output 												 
					if (token == NULL)
					 {						
						printf("ERROR, file input name expected\n");
					} else {
						int fd = open(token,  O_CREAT|O_APPEND | O_WRONLY, S_IRWXU);	//Open for creating,appending and writing
																						//	S_IRWXU  read, write, execute/search by owner
						if (fd == -1) 													//Failed open
						{
							printf("ERROR, open failed\n");
						} else {
							if (dup2(fd, 1) == -1) //redirect standard output to the file using dup2,the stdin or stdout file descriptors (0 and 1) 						
							{ 										
								printf("ERROR, dup2 failed\n");
							}
						}
					}				
				}else if(token[0] == '>' && token[1]=='\0'){ 						//redirect output
					token = strtok(NULL, delimiter);								//the next  token from  > has the name of the file output 											 
					if (token == NULL)
					 {						
						printf("ERROR, file input name expected\n");
					} else {
						int fd = open(token, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);	//Open for creating,truncating and writing
																						//	S_IRWXU  read, write, execute/search by	owner			
						if (fd == -1) 													//Failed open
						{
							printf("ERROR, open failed\n");
						} else {
							if (dup2(fd, 1) == -1) //redirect standard output to the file using dup2,the stdin or stdout file descriptors (0 and 1) 						
							{ 										
								printf("ERROR, dup2 failed\n");
							}
						}
					}
				} else if (token[0] == '|') {
					swlhnwsh++;															
					if (pipe(pipefds[swlhnwsh]) == -1) {								//Failed pipe											
				        	perror("pipe");
					        exit(EXIT_FAILURE);
 				 	 }
					cpid = fork();														//creation of a new child process
					if (cpid == -1) {													//Failed fork
						perror("fork");
						exit(EXIT_FAILURE);
					}
					if (cpid > 0) {  													//parent process reads from pipe 
						close(pipefds[swlhnwsh][1]);        							//Close unused write end of pipe
						if (dup2(pipefds[swlhnwsh][0], 0) == -1) { 						//redirects the keyboard to the file
							printf("ERROR, dup2 failed\n");
						}
						args = 0;
					} else {															//if you are the child process , break
						break;
					}
				} else {
					argv[args] = (char*)malloc(strlen(token) * sizeof(char)); 		//allocate memory for the argument
					sprintf(argv[args],"%s",token);									//is used to store formatted data as  a string
					args++;															//increase the arguments +1
					
				}
				token = strtok(NULL, delimiter);			//In each subsequent call that should  parse the same string, str must be NULL
			}
			if (cpid == 0) {														//child process writes to the pipe
				if (swlhnwsh >= 0) {
					close(pipefds[swlhnwsh][0]);         			 				// Close unused read end of pipe
					if (dup2(pipefds[swlhnwsh][1], 1) == -1) { 						//redirects the keyboard to the file
						printf("ERROR, dup2 failed\n");
					}
				}
			}
			argv[args] = NULL;															//Indicates the end of arguments. 
			int execution;
			execution = execvp(argv[0], argv);										//argv[0] has the entoli name,execution of the command
			if (execution == -1) {
				printf("ERROR, execvp failed at command %s!\n", entoli);
			}
			if (cpid != 0) {														
				if (swlhnwsh >= 0) {
					close(pipefds[swlhnwsh][0]);          								// Reader will see EOF 
					pid_t  waitPid, status;
					waitPid = wait(&status);											//and waiting for my child process to finish
					if (waitPid == -1) {												//Failed wait
						printf("ERROR, wait failed!\n");
						exit(-1);
					}             															 
				}
			} else {
				close(pipefds[swlhnwsh][1]);
			}
			
			exit(0);																//Finished successfully
		} else if (pid == -1) {														//Failed fork
			printf("ERROR, fork failed\n");
		}				
	}
	free(entoli);																	//deallocates the memory previously allocated by malloc
	printf("\n");
	return 0;
}

//pipefd[0] refers to the read end of the pipe. pipefd[1] refers to the write end of the pipe
//pipe() creates a pipe that can be used for interprocess communication



