#ifndef WEBSERVER_QUEUE_H
#define WEBSERVER_QUEUE_H
#include "assert.h"
#include "segel.h"
#include <sys/time.h>
//#include "stdio.h"
// queue is like this : dummy->TAIL->...->...->HEAD->NULL
struct queue{
    int process_fd;
    struct timeval  arrival;
    struct timeval  dispatch;
    struct queue* next;
};
typedef struct queue* Queue;
Queue create();
int getSize(Queue q);
void enqueue(Queue q,int process_fd,struct timeval arrival_time,struct timeval dispatch_time);  // push from tail
int dequeue(Queue q); // pop from head
void removeQ(Queue q, int process_fd);//remove specific instance
void destroy(Queue q); // destructor
int getHead(Queue q);
struct timeval getArrivalTime(Queue q,int process_fd);
//int getElementByIndex(Queue q, int k); // k is between 1 and queue_size
#endif