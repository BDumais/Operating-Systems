/*
 * Code written by Ben Dumais, 250669195
 * For CS3305b, 2016
 * Code heavily adapted from posted example of 'server3.c' from course website
 *   -> Uses same logic for creating server and accepting
 */
 
 //Includes
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <pthread.h>
#include <semaphore.h>

//#include "queue.c"

//Definitions
#define MAX_MSG_SIZE 1024

//Global Variables **THESE ARE SHARED BY THREADS**
int num_connections, max_connections;	//Variables for connections in array and maximum allowed connections
int *arr = NULL;	//Array of connections, tobo be initialized
sem_t sem;			//Semaphore, helps avoid busy waiting and synchronize access to global variables
	
/*
 * This function receives receives two numbers from the client and sends the sum back
 * Modified from 'server3.c'
 */
 
void proc_value(int connfd){
  int numbytes;
  char recvBuff[MAX_MSG_SIZE], sendBuff[MAX_MSG_SIZE];
  time_t ticks; 
  int a, ret;
  int rv;

  memset(recvBuff, '0',sizeof(recvBuff));
  memset(sendBuff, '0', sizeof(sendBuff)); 
 
  /*receive data from the client*/
  numbytes = recv(connfd,recvBuff,sizeof(recvBuff)-1,0);
  if (numbytes == -1){
       perror("recv");
       exit(1);
  }
  recvBuff[numbytes] = '\0';

  /*Extract the numbers */
  sscanf(recvBuff, "%d", &a);	//Read the number

  ret = a*10;	//Multiply by 10

  snprintf(sendBuff, sizeof(sendBuff), "%d", ret);	//Write to connection
  send(connfd, sendBuff, strlen(sendBuff),0); 	//Send

  close(connfd);	//Close connection
}
/*
 * Function represents one of the threads in server
 * Thread loops and enters the queue within semaphore. Once it is given access, it checks if there is a connection: if yes, it grabs the connection
 *   and updates counter before releasing semaphore and executing connection. This avoids busy waiting as threads are suspended until they
 *   leave the queue
 */
void proc_thread(){
	while(1){	//Loop
		int connfd = -1;
		sem_wait(&sem);	//Down semaphore (as we want to grab a connection)
		
		if (num_connections > 0){	//If there is a connection in the array, fetch one
			connfd = arr[num_connections-1];	//Fetch socket address from array
			num_connections--;	//Decrement pointer (so it points to next connection)
		}
		
		sem_post(&sem);	//Up semaphore (let next thread access data)
		if(connfd > 0)	//Check if we grabbed a connection from the queue
			proc_value(connfd);	//Process connection	

	}
}

//Main Function
int main(int argc, char *argv[]){
    int listenfd = 0, connfd = 0;
	int num_threads;
    struct sockaddr_in serv; 
    int rv;

    /*Command line argument: port number thread number connection max*/

    if(argc != 4)	//If improper arguments, prompt user
    {
        printf("\n Usage: %s port threads max_connections \n",argv[0]);
        return 1;
    }
	if ((num_threads = atoi(argv[2])) <= 0){	//If third argument is not positive integer, exit
		printf("\n Please enter a valid positive number for threads \n");
		return 1;		
	}
	if ((max_connections = atoi(argv[3])) <= 0){	//Check fourth argument
		printf("\n Please enter a valid positive number for max connections \n");
		return 1;
	}
	
	//Resize connection array
	arr = (int*) realloc(arr, max_connections * sizeof(int));
	
    /*Socket creation and binding*/

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd <  0) {
      printf("Couldn't make socket");
      return 1;
    }
    memset(&serv, '0', sizeof(serv));

    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    serv.sin_port = htons(atoi(argv[1])); 

    rv = bind(listenfd, (struct sockaddr*)&serv, sizeof(serv)); 
    if (rv <  0) {
      printf("Error binding");
      return 1;
    }
	
	//Create Threads
	int i;
	for(i=0;i<num_threads;i++){	//Loop until we create all desired threads
		pthread_t thr;	//Create pointer for thread
		pthread_create(&thr, NULL,(void *) &proc_thread, NULL);	//Create thread and have it run the proc_thread function
	}
	
	sem_init(&sem, 0, 1); //Initialize semaphore to value 1
	
    listen(listenfd, 10); 
	
    /*Accept connection and add to connection queue*/
	
    while(1)
    {
        if(num_connections < max_connections){
			
			connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); //Accept a connection
			
			sem_wait(&sem);	//Down semaphore (wait until we get access)
			
			arr[num_connections] = connfd;	//Take socket from Accept and place into array at open spot
			num_connections++;	//Increment num_connections
			
			sem_post(&sem);//Up semaphore
			
			//printf("\nconnection added");
			
		}
		//sleep(1);
     }
	 
	 sem_destroy(&sem); //Destroy semaphore
}
