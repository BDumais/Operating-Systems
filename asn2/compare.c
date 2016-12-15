/*
 * compare.c written by Ben Dumais for CS3305, Assignment 2
 * 250669195, bdumais
 */
 
 //Includes
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>

//Definitions
#define BILLION  1000000000L
#define CLOCK_ID CLOCK_PROCESS_CPUTIME_ID

/*Timing functionality borrowed from posted example for Assignment 2 */

uint64_t diff(struct timespec start, struct timespec end)
{
  uint64_t diff;

  diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;	//Calculate difference

  return diff;
}

//Minimum function, does nothing
void minFunction(){
	return;
}

//Main function
int main(int argc, char *argv[])
{
  struct timespec start, end;
  uint64_t elapsedTimeSys, elapsedTimeFun;
  int i,s;

  clock_gettime(CLOCK_ID, &start);	//Start timer
  getpid();	//Run System Call
  clock_gettime(CLOCK_ID, &end);	//End timer
  
  elapsedTimeSys = diff(start,end);	//Calculate difference for system call
  
  clock_gettime(CLOCK_ID, &start);	//Repeat for function call
  minFunction();
  clock_gettime(CLOCK_ID, &end);
  
  elapsedTimeFun = diff(start,end);
  
  //Print results
  printf("System: %llu\nFunction: %llu\n",(long long unsigned int) elapsedTimeSys, (long long unsigned int) elapsedTimeFun);

}