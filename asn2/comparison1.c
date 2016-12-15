/*
 * comparison1.c written by Ben Dumais for CS3305, Assignment 2
 * 250669195, bdumais
 */

//Includes
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <string.h>
#include <stdlib.h>
 
//Main function
int main(int argc, char *argv[]){
	
	
	//Following code adapted from posted example
	int policy;
    struct sched_param param;
   
    if (argc != 2){
       perror("Please enter 2 arguments\n");
       exit(0);
    }

    if (!strcmp(argv[1], "SCHED_OTHER")) {
         policy = SCHED_OTHER;
    }
    else if(!strcmp(argv[1], "SCHED_FIFO")){
         policy = SCHED_FIFO;
    }
    else if (!strcmp(argv[1], "SCHED_RR")){
         policy = SCHED_RR;
    }
    else{
         perror("Invalid Policy, exiting\n");
         exit(0);
    }
    

     param.sched_priority = sched_get_priority_max(policy);    
     if(sched_setscheduler(0,policy,&param)){
         perror("Error setting policy, exiting\n");
         exit(0);
     }	
	
	//Loop forever
	while(1){
		
		sleep(1);	//Sleep
		
		int pid;		//Upon return, fork program
		pid = fork();
		if(fork==0){	//In child, run cpuTimeWaste
			execl("./cpuTimeWaste", "./cpuTimeWaste", (char*)NULL);
			printf("Error Executing\n");	//Print error if we could not execute
		}
	}
}
