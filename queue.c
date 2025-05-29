//
#include "queue.h"
#include <stdio.h>
#define DUMMY_FD -1
//init: queue = dummy ->NULL;
//inserting A, for example, before : dummy -> TAIL->...->HEAD->NULL;
//after inserting : dummy->A->OLD_TAIL->..->HEAD->NULL
//queue is like this : dummy->TAIL->...->...->HEAD->NULL
sem_t semaphore;
pthread_mutex_t m;
Queue create() {
    sem_init(&semaphore,0,0);
    
    Queue dummy;
    dummy = (Queue) malloc(sizeof(struct queue));
    if (dummy == NULL) {
        //unix_error("Malloc error");
        exit(1);
    }
    else{
        dummy->next=NULL;
        gettimeofday(&dummy->arrival, NULL);
        gettimeofday(&dummy->dispatch, NULL); //filler value, doesn't rperesent the actual dispatch time.
        dummy->process_fd = DUMMY_FD;
    }
    return dummy; //this is a pointer to struct queue
}
int getSize(Queue q){
     
    int count = -1;
    Queue temp = q;
    while (temp != NULL) {
        temp = temp->next;
        count++;
    }
     
    return count;
}
void enqueue(Queue q,int process_fd,struct timeval arrival_time,struct timeval dispatch_time){
     
    assert(q->process_fd == DUMMY_FD);
    Queue to_insert= (Queue)malloc(sizeof(*to_insert));
    if(to_insert == NULL){
       // unix_error("Malloc error");
        exit(1);
    }
    to_insert->arrival = arrival_time;
    to_insert->process_fd = process_fd;
    to_insert->dispatch = dispatch_time;
    to_insert->next = q->next;
    q->next = to_insert;
     
}  // push from tail
// pop from head
int dequeue(Queue q) {
    
    printf("ok\n");
    assert(q!= NULL && q->process_fd == DUMMY_FD);
     
    if (q->next == NULL) {
         
        return -1;
    }
    Queue temp = q;
    Queue to_delete;
    int head_fd;
    while(temp->next->next != NULL) {
        temp = temp->next;
    }
    to_delete = temp->next ;
    temp->next = NULL; //the deletion
    head_fd = to_delete->process_fd;
    free(to_delete);

    return head_fd; // to be changed if needed... we may need to add parameters that we will fill once we fidn the head,
    // for example to pass them to the RequestHandler method, i.e: ArrivalTime,DispatchTime and so on..
}
//init: queue = dummy ->NULL;
//inserting A, for example, before : dummy -> TAIL->...->HEAD->NULL;
// after inserting : dummy->A->OLD_TAIL->..->HEAD->NULL
// queue is like this : dummy->TAIL->...->...->HEAD->NULL
void removeQ(Queue q, int process_fd) {//remove specific instance
    Queue temp = q;
    Queue prev = q;
    if (q->next != NULL){
        while (temp != NULL) {
            if (temp->process_fd == process_fd) {
                prev->next = temp->next;
                temp->next = NULL;
                free(temp);
                break;
            }
            prev = temp;
            temp = temp->next;
        }
    }

}
void destroy(Queue q) { // destructor
    Queue to_delete = NULL;
    Queue runner = q;
    while(runner!=NULL)
    {
        to_delete = runner;
        runner = runner->next;
        to_delete->next = NULL;
        free(to_delete);
    }

    return;
}
int getHead(Queue q) {
    if (q->next == NULL) {
        return q->process_fd;
    }
    Queue temp = q;
    while (temp->next != NULL) {
        temp = temp->next;
    }

    return temp->process_fd;
}

struct timeval getArrivalTime(Queue q,int process_fd) {
    struct timeval required_timeval;
    timerclear(&required_timeval);
    Queue temp = q;
    while (temp != NULL) {
        if (temp->process_fd == process_fd) {
            return temp->arrival;
        }
        temp = temp->next;
    }
    return required_timeval;
}
/*
int getElementByIndex(Queue q, int k) { // k is between 1 and queue_size
    assert(k>0);
    Queue temp = q;
    while (k > 0) {
        temp = temp->next;
        k--;
    }
    return temp->process_fd;
}*/
