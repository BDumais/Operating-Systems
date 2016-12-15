/*
 * simulator.c Written by Ben Dumais for CS3305 Assignment 4
 * 250669195, bdumais
 */

 //Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//Definitions
#define MAX 1024
#define true 1
#define false 0
#define DEBUG 0

//Structs

/*	Struct representing a page	*/
typedef struct page{
	int id;		//Id of page	
	int frame;	//Id of frame page is loaded into (if any)
	int valid;	//Boolean value for if it is valid
	int t_val;	//Value storing how long page has been in a frame
	int f_val;	//Value counting a page's frequency of use
}pageEntry;

/*	Struct representing a frame	*/
typedef struct frame{
	int id;			//Id of frame
	pageEntry *p;	//Page id currently in frame
}frameEntry;

//Global Variables
frameEntry *frameTable;
pageEntry *pageTable;
int numFrames;
int activeFrames;

//Functions


/*	LRU algorithm, uses counters to compare value of page	*/
void LRU(pageEntry *page){
	
	int lru_p = -1;	//Initialize id of page to swap
	int max = -1;	//Set min t_val to a small number
	int i;			
	for(i=0; i<numFrames; i++){	//Loop through frame table
		int t = frameTable[i].p->t_val;
		if(t > max){	//If the current t_val is greater than max
			lru_p = i;		//Update page to swap
			max = t;		//Update Max
		}
	}
	if(DEBUG)
		printf("\tSwap page %d for page %d\n",frameTable[lru_p].p->id,page->id);
	
	frameTable[lru_p].p->frame = -1;	//Set frame to -1
	frameTable[lru_p].p->t_val = 0;		//Reset t_val
	frameTable[lru_p].p->valid = false;	//Update oldest page to invalid
	
	frameTable[lru_p].p = page;			//Swap oldest page with input
	frameTable[lru_p].p->t_val = 0;		//Reset t_val of new page
	page->valid = true;					//Update new page to be valid (in a frame)
	
}

/*	LFU algorithm, finds min f_val and swaps	*/
void LFU(pageEntry *page){
	
	int lfu_p = -1;	//Initialize id of page to swap
	int min = MAX;	//Set max f_val to a large number
	int i;			
	for(i=0; i<numFrames; i++){	//Loop through frame table
		if(frameTable[i].p->f_val < min){	//If the current t_val is greater than max
			lfu_p = i;					//Update page to swap
			min = frameTable[i].p->f_val;//Update min
		}
	}
	
	frameTable[lfu_p].p->frame = -1;		//Set frame to -1
	frameTable[lfu_p].p->valid = false;	//Update oldest page to invalid

	if(DEBUG)
		printf("\tSwap page %d for page %d\n",frameTable[lfu_p].p->id,page->id);
	
	frameTable[lfu_p].p = page;			//Swap oldest page with input
	page->f_val++;		//Increment f_val of new page, as it is being used
	page->valid = true;	//Update new page to be valid (in a frame)
	
}

/*	Method to update t_vals of all active pages	*/
void updateTVals(){
	int i=0;
	//Loop until we hit all frames or a frame with no page
	while(i < numFrames && i < activeFrames){
		frameTable[i].p->t_val++;	//Increment t_val
		i++;
	}
}



/*	Main function	*/
int main(int argc, char *argv[]){
	if(argc != 4){	//Check number of arguments, exit if incorrect
		printf("Usage: simulator num_frames file method\n");
		return -1;
	}
	
	//Get number of frames from arguments
	numFrames = atoi(argv[1]);
	if(numFrames <= 0){	//If non positive, exit
		printf("Please enter a positive integer for number of frames\n");
		return -1;
	} 
	
	//Check that user has input proper method, ie LRU of LFU
	if(strcmp("LRU", argv[3]) != 0 && strcmp("LFU", argv[3]) != 0){ //Exit if not
		printf("Please enter a valid method (LFU or LRU)\n");
		return -1;
	}
	
	
	FILE *f = fopen(argv[2], "r");	//Open the input file
	
	if(f == NULL){ //If unable to open, print message and exit
		printf("Failed to read from %s\n", f);
		return -1;
	}
	
	int faults = 0;			//Initialize number of faults variable to 0
	activeFrames = 0;	//Initialize number of used frames
	
	//Allocate space for pages and frames
	frameTable = (frameEntry*)malloc(sizeof(frameEntry)*numFrames);
	pageTable = (pageEntry*)malloc(sizeof(pageEntry)*MAX);
	
	int i;
	//Initialize values in frame tables
	for(i=0;i<numFrames;i++){		//Loop
		frameTable[i].id = i;		//Set id
		//frameTable[i].p->id = -1;	//Set id of page to null
	}

	//Initialize values for pages in table
	for(i=0;i<MAX;i++){
		pageTable[i].frame = -1;	//Set frames to -1, ie no frame
		pageTable[i].valid = false;	//Start at invalid
		pageTable[i].t_val = 0;		//Set variables to 0
		pageTable[i].f_val = 0;
		pageTable[i].id = i;		//Set id of frame (used for tracking)
	}
	
	int page; //String to hold current memory address from file
	//size_t s = 0;
	
	if(strcmp("LRU", argv[3]) == 0){	//If input mode is LRU, use that policy
		while(fscanf(f, "%d", &page) != EOF){
				
				//Check if adr loaded and page is valid
				if(pageTable[page].valid == false){	//Check if page is not loaded (valid)
					
					faults++;		//If its not, increment number of faults
					//If there are free frames
					if(activeFrames < numFrames){
						frameTable[activeFrames].p = &pageTable[page];	//Set page of lowest available frame to current page
						pageTable[page].valid = true;		//Set current page to valid
						pageTable[page].frame = activeFrames; //Set frame id of current page
						activeFrames++;						//Increment activeFrames
						if(DEBUG)
							printf("\tAdd page %d\n",page);
					}
					
					//If there are no available frames, select a replacement via LRU algorithm
					else{	
						LRU(&pageTable[page]);
					}
				}
				else{
					if(DEBUG)
						printf("\tPage %d in frame already\n", page);
					pageTable[page].t_val = 0;	//Reset t_val of current page if it is in a frame already
				}
				updateTVals();	//Update the t_vals of all pages

		}//end of while
	}
	else{	//Otherwise they input LFU, so use that policy
		while(fscanf(f, "%d", &page) != EOF){

				//Check if adr loaded and page is valid
				if(pageTable[page].valid == false){	//Check if page is not loaded (valid)
					faults++;		//If its not, increment number of faults
					
					//If there are free frames
					if(activeFrames < numFrames){			
						frameTable[activeFrames].p = &pageTable[page];	//Set page of lowest available frame to current page
						pageTable[page].valid = true;		//Set current page to valid
						pageTable[page].frame = activeFrames;	//Set frame id of current page
						frameTable[activeFrames].p->f_val++;	//Increment frequency value of current page
						activeFrames++;						//Increment activeFrames
						if(DEBUG)
							printf("\tAdd page %d\n",page);						
					}
					
					//If there are no available frames, select a replacement via LRU algorithm
					else{	
						//printf("Entering LFU\n");
						LFU(&pageTable[page]);
					}
				}
				else{	//If page is already in a frame (ie it is valid), just update its f_val
					if(DEBUG)
						printf("\tPage %d in frame already\n", page);
					pageTable[page].f_val++;
				}
		}//end of while
	}
	
	fclose(f);	//Close file
	
	//Print number of faults and exit
	printf("Page faults: %d\n", faults);
	return 0;
	
}
