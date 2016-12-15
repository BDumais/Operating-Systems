/*
 * comparison1.c written by Ben Dumais for CS3305, Assignment 2
 * 250669195, bdumais
 */
 
//Functions
void sigHandler(int sig);
 
//Main function
int main(int argc, char *argv[]){
	
	signal(SIGINT, sigHandler);	//Initialize signal handler
	
	while(1){
		
		sleep(1);
		
		int pid;
		pid = fork();
		if(fork==0){
			execl("/CS3305/Asn2/cpuTimeWaste", "/CS3305/Asn2/cpuTimeWaste", (char*)NULL);
		}
	}
}

//Signal Handler, terminates program
void sigHandler(int sig){
	printf("Comparison process killed");
	return;
}