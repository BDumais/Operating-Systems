/*
 * kernelInfo.c written by Ben Dumais for CS3305, Assignment 2
 * 250669195, bdumais
 */

 //Includes
#include <stdio.h>
#include <string.h>

//Definitions
#define MAX 1024

//Main function
int main(int argc, char *argv[])
{
	char *cpu = "/proc/cpuinfo";	//strings to store location of needed files
	char *kern = "/proc/version";
	
	char buff[MAX];	//Buffer for file reading
	char *cpuType;	//String to store type of cpu
	char *kernVer;	//String to store kernel version
	size_t s = 0;	//Size of buffer
	
	FILE *cpuFile;	//File pointers to respective files
	FILE *kernFile;
	
	//Read contents of cpuinfo into buffer
	cpuFile = fopen(cpu, "r");
	s = fread(buff, 1, MAX, cpuFile);
	fclose(cpuFile);
		
	//If size is 0, we failed to read	
	if(s == 0){
		printf("Failed to read from %s", cpu);	//Print error and exit
		return;
	}
	
	buff[s] = '\0';	//Terminate string with null
	cpuType = strstr(buff, "model name");	//Fetch line with model name
	cpuType = strtok(cpuType, ":");			//Remove text before colon
	cpuType = strtok(NULL, "\n");			//Remove text after new line (leaving us with only the type)
	printf("CPU type:\t%s\n", cpuType);		//Print cpu type
	
	//Repeat process with kernel version
	kernFile = fopen(kern, "r");
	s = fread(buff, 1, MAX, kernFile);
	fclose(kernFile);
	
	if(s == 0){
		printf("Failed to read from %s", kern);
		return;
	}
	buff[s] = '\0';
	kernVer = strtok(buff, ")");
	printf("Kernel version:\t%s)\n", kernVer);
}