#include "queue.h"

struct QueueStruct{
	int front, count, size;
	int contents[];
};

typedef struct QueueStruct *Queue;

QueueStruct InitQueue(int s){
	struct QueueStruct *q = malloc(sizeof(struct QueueStruct) + s);
	q->front = 0;
	q->count = 0;
	q->size = s;
	return(q);
}

void Insert(int k){
	if(q->count < size)
		q->contents[count++] = k;
	
}
int Delete(){
	if(q->count==0)
		return -1;
	int i = q->contents[0];
	int j;
	for(j=0;j < q->size - 1;j++)
		q->contents[j]=q->contents[j+1];
	q->count--;
	return i;
}