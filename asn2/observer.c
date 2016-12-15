/*
 * observer.c written by Ben Dumais for CS3305, Assignment 2
 * 250669195, bdumais
 */
 
//Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

//Defines
#define MAX 1024

//Functions
void sigHandler(int sig);
 
//Main function
int main(int argc, char *argv[]){

	signal(SIGINT, sigHandler);	//Initialize signal handler
	
	if(arv[1]==NULL){
		printf("No input, exiting\n");
		exit;
	}
	
	char *proc = argv[1];	//Get passed argument to program
	
	int pid;				//Pid of current process	
	pid = fork();			//Fork program
	
	if(pid < 0){		//On fork error, exit
		printf("Unable to fork, exiting\n");
		exit(0);
	}
	else if(pid==0){	//Child
		execlp(proc, proc, (char *)NULL);	//Execute process that was input through parameters
	}
	else{				//Parent

		int status;			//Status variable, for waiting
		
		char *utimeLine;	//Strings to contain utime and stime Lines, which will be parsed from file
		char *stimeLine;
		
		char fPath[MAX] = {0};		//File path string
		char buff[MAX];		//Buffer for reading file
		size_t s = 0;		//Size for buffer
	
		FILE *pidStats;		//File path variable
		
		//Create file path variable by adding pid into /proc/<pid>/stat
		sprintf(fPath, "/proc/%d/stat", pid);
		
		//Loop until child process finishes
		while(waitpid(pid, &status, WNOHANG) == 0){	
			pidStats = fopen(fPath, "r");		//Open pid stats
			s = fread(buff, 1, MAX, pidStats);	//Read into buffer
			fclose(pidStats);					//Close file
				
			buff[s] = '\0';						//Add null to input
			int i;					//Loop variable
			strtok(buff, " ");		//Start tokenizing buffer
			for(i=0;i<13;i++)		//Loop 13 times (gets us to utime value)
				strtok(NULL, " ");
			
			utimeLine = strtok(NULL, " ");	//Get next token, which is utime
			stimeLine = strtok(NULL, " ");	//Get next token, this time stime
			printf("User Time: %s\tKernel Time: %s\n",utimeLine, stimeLine);	//Print values
				
			sleep(1);	//Sleep for 1 second

		}//end while
	}//end else
	

}//end main


//Signal Handler, terminates program
void sigHandler(int signum){
	printf("Observer process stopped");
}
