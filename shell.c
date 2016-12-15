/*
 * Ben Dumais, 250669195
 * 3305b, Spring 2016
 * Assignment 1
 */

 //Includes
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

//Definitions
#define MAX_SIZE 256	//Max size of a str
#define MAX_CMD 10		//Maximum amount of commands in a line
#define EMPTY -1		//Indicates an empty pipe or failed child

#define DEBUG 0			//Debug mode - Change to 1 to show history debugging
	
//Method Declarations
int splitCommands(char *line, char *comms[]);
int tokenize(char *line, char *tokens[]);
pid_t execute(char *args[], int pipes[][2], int n);
void closePipes(int pipes[MAX_CMD][2]);
void resetPipes(int pipes[MAX_CMD][2]);

void sigHandler(int);

void updateHistory(char *input, char *history[]);
void printHistory(char *history[]);
void initHistory(char *history[]);


//Main Function
int main(void){
	
	signal(SIGINT, sigHandler);	//Start signal handler
	
	/* Declare variables */
	
	char str[MAX_SIZE];		//Variable for input string (will be tokenized)
	char str2[MAX_SIZE];	//Variable for second input string (will not be tokenized)
	
	char *commands[MAX_CMD];//Variable for commands array, stores commands
	char *args[MAX_CMD];	//Variable for arguments array, stores arguments for a command
	char *hist[MAX_CMD];	//Variable for history array, stores previous commands

	char *exitString = "exit\n";		//String storing the exit command
	char *historyString = "history\n";	//String storing the history command
	
	int numCom;				//Variable to store amount of different commands user has input
	int numArgs;			//Variable to store amount of arguments for a command
		
	pid_t pids[MAX_CMD];	//Variable to store PIDS from commands being run
	int pipes[MAX_CMD][2];	//Variable to store pipes for each process
	
	
	resetPipes(pipes);	//Initialize pipes to all be empty
	initHistory(hist);	//Initialize History array (so no memory errors occur)
		
	while(1) {	//Loop forever
		printf("\n3305Asn1/ben-dumais>");	//Print prompt
			
		fgets(str,sizeof(str),stdin);	//Read in string
				
		if(strcmp(str,exitString) == 0)	//If user input "exit", exit program
			return 0;
		else if(strcmp(str,historyString) == 0)	//If user input "history", print history array
			printHistory(hist);
		
		//Check for a < or >, indicating IO is needed	
		else if(strchr(str,'<')!= NULL || strchr(str,'>')!=NULL){	//Search for those characters, and if either return with a valid pointer in the string, we need to do IO
			
			char *in = strchr(str,'<');		//Get pointer to <, if it exists
			char *out = strchr(str,'>');	//Get pointer to >, if it exists
			
			int toks = splitCommands(str, commands);	//Split input into commands

			if(toks == 3){	//If there are three commands, it has both in and out redirection - We assume the command < in > out format
				int pid;
				pid = fork();	//Fork 
				if(pid==0){				//In child, setup redirection and execute
					FILE *fp1 = fopen(commands[1],"r");	//Open the first file as read
					FILE *fp2 = fopen(commands[2], "w"); //Open the second file as write
					dup2(fileno(fp1), STDIN_FILENO);	//Redirect both stdin and stdout to appropriate places
					dup2(fileno(fp2), STDOUT_FILENO);
					fclose(fp1);						//Close both files
					fclose(fp2);
					tokenize(commands[0], args);		//Tokenize the actual command (ie, ls -l)
					execvp(args[0], args);				//Execute
				}
				else{
					int status;
					waitpid(pid, &status, 0);
				}
			}
			else if(toks == 2){	//If there were 2 tokens, it is either command < in or command > out
				if(in != NULL){		//If there is a pointer to a < character, we need to redirect input
					int pid;
					pid = fork();	//Fork
					if(pid==0){
						FILE *fp = fopen(commands[1],"r");	//Open file
						dup2(fileno(fp), STDIN_FILENO);		//Redirect
						fclose(fp);							//Close
						tokenize(commands[0], args);		//Tokenize command
						execvp(args[0], args);				//Execute
					}					
					else{
						int status;
						waitpid(pid, &status, 0);
					}	
				}
				else{			//Otherwise it was a > character, so redirect output	
					int pid;
					pid = fork();	//Same as before: fork, open, redirect, close, execute
					if(pid==0){
						FILE *fp = fopen(commands[1],"w");
						dup2(fileno(fp), STDOUT_FILENO);
						fclose(fp);
						tokenize(commands[0], args);
						execvp(args[0], args);
					}
					else{
						int status;
						waitpid(pid, &status, 0);
					}

				}
				
				updateHistory(str,hist);	//Add original string into history array
			
			}
			else	//If tokens is neither 2 or 3, the input contained a < or > but is not supported
				printf("\nERROR: invalid IO command. Expected format: _command_ < _input_ > _output_");	//Print a reminder of corrent format
			
			
		}
		else{	//Otherwise, run the command(s)
			
			if(DEBUG){	//Debugging for history array
				printf("\nHistory pre split:\n");
				printHistory(hist);
			}
			
			strncpy(str2,str, MAX_SIZE);		//Copy string into str2 variable, (for use in history)
			numCom=splitCommands(str,commands); //Split string into commands (splits by | or new line)
												//Number of commands is stored
				
			if(DEBUG){	//Debugging for history array
				printf("\nHistory post split:\n");
				printHistory(hist);
			}		//As it can be seen from debugging, the history completely changes after tokenization
			
			int i;
			for(i=0;i<numCom - 1; i++){		//for each command user wants to run, we need to create pipes (num commands -1, as each pipe connects 2)
				
				int p[2];	//Create pipes
				pipe(p);
				
				//Now we designate the input and output 
				pipes[i][STDOUT_FILENO] = p[STDOUT_FILENO];	//Set the output of current command to input of pipe (store output for next operation)
				pipes [i+1][STDIN_FILENO] = p[STDIN_FILENO];//Set the input of next command to output of pipe 	(take input for next operation)
			}
			
			//Loop through each command and execute in order
			for(i=0;i<numCom;i++){
				tokenize(commands[i],args);			//Split current command into its arguments
				pids[i] = execute(args,pipes,i);	//Execute command and place PID into array
			}
					
			closePipes(pipes);	//Close all pipes that were created
			
			for(i=0;i<numCom;i++){	//Loop through commands, waiting for each to terminate
				int status;
				waitpid(pids[i], &status, 0);
			}		
			
			resetPipes(pipes);			//Reset all pipes to empty
			updateHistory(str2,hist);	//Add original string into history array
			
		}//End of Else strcmp
			
	}//End of While	

}//End of Main

//Methods

/*
 * Tokenizing code adapted from posted example for Assignment 1
 */

/* splitCommands method, takes string and array of strings, outputs number of tokens as integer
 * Splits input line into seperate commands (delimiter is |)
 * Used to split entire input into seperate commands */
int splitCommands(char *line, char *comms[]){
	int commands=0;	//initalize number of commands
	
	comms[commands]=strtok(line, "<>|\n");	//Get token and place in first position
	do{	//Loop until we read a null
		commands++;
		comms[commands]=strtok(NULL, "<>|");	//Tokenize and add to array
	} while(comms[commands] != NULL);

	return commands;	//Return commands
}
 
/* tokenize method, takes a string and array of strings, outputs integer of amount of tokens
 * Splits input line into arguements (delimits on spaces and newline)
 * Used to break a single command into its arguments */
int tokenize(char *line, char *tokens[]){
	int i=0;

	tokens[i]=strtok(line, " \n");	//Tokenize
	do {	//Loop until we read a null
		i++;
		tokens[i]=strtok(NULL, " \n");	//Tokenize and add to token array
	} while(tokens[i] != NULL);
	
	tokens[++i] = NULL;	//Terminate with a null (required by execvp)
	i--;		//Decrement to match number of tokens
	return i;	//Return number of tokens
}

	
/* execute method, takes array of string, pipes array, and an integer, outputs a pid
 * Takes a list of arguments, pipes it accordingly, and executes the indicated process
 * Used to execute all commands input by user */
pid_t execute(char *args[], int pipes[][2], int n){
	
	pid_t pid;
	int i;
	
	pid = fork();	//Fork process
					//This creates a child that runs exec, which will then close after completion
	
	if(pid==0){
		//Designate where to direct input and output
		if(pipes[n][STDIN_FILENO] >= 0)		//If current pipe is positive
			dup2(pipes[n][STDIN_FILENO], STDIN_FILENO);	//replace input for this pipe with new fd
		
		if(pipes[n][STDOUT_FILENO] >= 0)	//Repeat for STDOUT
			dup2(pipes[n][STDOUT_FILENO], STDOUT_FILENO);
		
		
		closePipes(pipes);	//Close pipe ends 
		
		execvp(args[0],args);	//Execute the command
		
	}//End of fork
	return pid;
}	
/* closePipes method, takes pipes array, no output
 * Loops through pipes and closes them */
void closePipes(int pipes[MAX_CMD][2]){
	int i;
	for(i=0;i<MAX_CMD;i++){	//Loop through all pipes
	
	if(pipes[i][STDIN_FILENO] >= 0)		//If a pipe's input is positive, close it
		close(pipes[i][STDIN_FILENO]);
	if(pipes[i][STDOUT_FILENO] >= 0)	//If a pipes's output is positive,close it
		close(pipes[i][STDOUT_FILENO]);
		
	}
}
	
/* resetPipes method, takes pipe array, no output
 * Loops through pipe array and resets all pipes to empty */
void resetPipes(int pipes[MAX_CMD][2]){
	int i;
	for(i=0;i<MAX_CMD;i++){
		pipes[i][STDIN_FILENO] = EMPTY;
		pipes[i][STDOUT_FILENO] = EMPTY;
	}
}	
	
/* sigHandler method, takes an integer
 * Used to interrupt program */
void sigHandler(int signum){
	printf("\nInterrupted");
}	


//History methods	

/* At this point, history does not work as expected. For some reason, each time a
 * string is added into the array, all other entries are updated to the string that was
 * just added. This is not to do with the algorithm, but rather the strings themselves
 * change. I did not have time to debug properly (likely an issue with pointers)
 * To show the problem, change DEBUG to 1. As soon as the tokenize command is called,
 * the history array is changes, even though no direct modification was made to it */
	
/* updateHistory method, takes a string and array of string, no output
 * Method to update history (add most recent command) */
void updateHistory(char *input, char *history[]){
	int i;
	for(i=MAX_CMD-1;i>0;i--){	//Loop from back to front of array
		history[i]=history[i-1];	//Copy preceeding element into current
	}
	history[i]=input;	//Add most recent command to front
}

/* printHistory method, take array of strings, no output
 * Method to print history array */
void printHistory(char *history[]){
	int i=0;
	while(history[i] != NULL){	//Loop until we get a null
		printf("%s",history[i]);	//Print current string
		i++;
	}
}

/* initHistory method, takes array of strings, no output
 * Method to initalize history array to all null values */
void initHistory(char *history[]){
	int i;
	for(i=0;i<MAX_CMD;i++)	//Loop though all of array
		history[i]=NULL;	//Set each element to null
}