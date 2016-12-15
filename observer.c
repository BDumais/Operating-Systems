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
	
	char *proc = argv[0];	//Get passed argument to program
	
	pid_t pid;	//Pid of current process	
	pid = fork();			//Fork program
	
	if(pid < 0){		//On fork error, exit
		printf("Unable to Fork\n");
		return;
	}
	else if(pid==0){	//Child
		//printf("in child\n");
		signal(SIGINT, sigHandler);	//Initialize signal handler
		execlp(proc, proc, (char *)NULL);	//Execute process that was input through parameters
	}
	else{				//Parent

		int status;			//Status variable, for waiting
		
		char *utimeLine;	//Strings to contain utime and stime Lines, which will be parsed from file
		char *stimeLine;
		
		char *fPath;		//File path string
		char buff[MAX];		//Buffer for reading file
		size_t s = 0;		//Size for buffer
	
		FILE *pidStats;		//File path variable
		
		//Create file path variable by adding pid into /proc/<pid>/stat
		fPath += sprintf(fPath, "%s", "/proc/");
		fPath += sprintf(fPath, "%ld", (long)pid);
		fPath += sprintf(fPath, "%s", "/stat");
		
		//Loop until child process finishes
		while(waitpid(pid, &status, WNOHANG) == 0){	
			//printf("child is running\n");
			pidStats = fopen(fPath, "r");		//Open pid stats
			s = fread(buff, 1, MAX, pidStats);	//Read into buffer
			fclose(pidStats);					//Close file
				
			buff[s] = '\0';						//Add null to input
			utimeLine = strstr(buff, "utime");	//Find utime
			utimeLine = strtok(utimeLine, "\n");	//Tokenize at utime
				
			stimeLine = strstr(buff, "stime");	//Repeat for stime
			stimeLine = strtok(stimeLine, "\n");
				
			printf("%s\t%s\n",utimeLine, stimeLine);	//Print values
				
			sleep(1);	//Sleep for 1 second

		}//end while
	}//end else
	

}//end main


//Signal Handler, terminates program
void sigHandler(int signum){
	printf("Observer process stopped");
}
