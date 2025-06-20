#include "segel.h"
#include "request.h"
#include "log.h"
#include "queue.h"

//
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//
pthread_mutex_t queue_mutex;
pthread_cond_t master_condition;
pthread_cond_t worker_condition;
// global log
server_log requests_log;
//global pthread array
pthread_t* pthread_array; 
//global queue's , waiting_task_queue : queue for tasks enqueued from server(incoming clients), working_task_queue - queue used by worker threads to handle execution of requests.
Queue waiting_tasks_queue, working_tasks_queue;
// Parses command-line arguments
int queue_size;

void getargs(int *port,int *num_threads,int* queue_size, int argc, char *argv[])
{
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <port> <threads> <queue_size>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *num_threads = atoi(argv[2]);
    if(*num_threads<=0){
        app_error("invalid threads argument\n");
        exit(1);
    }
    *queue_size = atoi(argv[3]);
    if(*queue_size<=0){
        app_error("invalid queue_size argument\n");
        exit(1);
    }

}
// TODO: HW3 — Initialize thread pool and request queue
// This server currently handles all requests in the main thread.
// You must implement a thread pool (fixed number of worker threads)
// that process requests from a synchronized queue.
void* thread_function(void* arg/* the real argument is thread_stats* stats */){
    threads_stats t_stats = *((threads_stats*)arg); // we make conversion of arg to (thread_stats*), and then we dereference it to get thread_stats object.
    while (1) { 
        //printf("locking mutex first time in thread number %d \n", t_stats->id);
        pthread_mutex_lock(&queue_mutex);
        //printf("locking mutex first time in thread number %d success\n", t_stats->id);
        while(!(getSize(waiting_tasks_queue)>0)) 
        {
           // printf("checking master_condition in thread number %d \n", t_stats->id);
        // if (getSize(waiting_tasks_queue) +getSize(working_tasks_queue) < queue_size) 
        // {
            //printf("signaling master to be free thread number %d \n", t_stats->id);
            // pthread_cond_signal(&master_condition); /\/doubts about this line SALEEM */
            //printf("signaling master to be free thread number %d success\n", t_stats->id);
        // }
            //printf("waiting worker condition in thread number %d, size of waiting queue is %d, size of working queue"
            //        " is %d, lastly, queue size : %d \n", t_stats->id,getSize(waiting_tasks_queue),getSize(working_tasks_queue),queue_size);
            pthread_cond_wait(&worker_condition,&queue_mutex); // stuck here...
            //printf("waiting worker condition in thread number %d success\n", t_stats->id);
        }
        int fd = getHead(waiting_tasks_queue);
        struct timeval arrival_time = getArrivalTime(waiting_tasks_queue, fd);
        //printf("thread no.%d now dequeues from waiting_task_queue \n", t_stats->id);
        dequeue(waiting_tasks_queue);
        //printf("thread no.%d successfully dequeued from waiting_task_queue \n", t_stats->id);
        struct timeval dispatch_time;
        gettimeofday(&dispatch_time,NULL);
        struct timeval dispatch_interval ;
        timersub(&dispatch_time,&arrival_time,&dispatch_interval);
        //printf("thread no.%d now enqueues to working_tasks_queue \n", t_stats->id);
        enqueue(working_tasks_queue,fd,arrival_time,dispatch_interval);
        //printf("thread no.%d successfully enqueued to working_tasks_queue \n", t_stats->id);
        //printf("unlocking mutex first time in thread number %d\n", t_stats->id);
        pthread_mutex_unlock(&queue_mutex);
        //printf("unlocking mutex first time in thread number %d success\n", t_stats->id);
        requestHandle(fd, arrival_time, dispatch_interval, t_stats, requests_log);
        //printf("locking mutex second time in thread number %d \n", t_stats->id);
        pthread_mutex_lock(&queue_mutex);
        //printf("locking mutex second time in thread number %d success\n", t_stats->id);
        removeQ(working_tasks_queue,fd);
        if (getSize(waiting_tasks_queue) + getSize(working_tasks_queue) < queue_size) {
            //printf("pthread_cond_signal master_condition in thread number %d  \n", t_stats->id);
            pthread_cond_signal(&master_condition);
            //printf("pthread_cond_signal master_condition in thread number %d success \n", t_stats->id);
        }
        //printf("unlocking mutex second time in thread number %d \n", t_stats->id);
        pthread_mutex_unlock(&queue_mutex);
        //printf("unlocking mutex second time in thread number %d success \n", t_stats->id);
        Close(fd);
        }
        return NULL;
}
int main(int argc, char *argv[])
{
    int cntcnt = 0;
    /* printf("my pid is %d \n",(int)getpid()); */
    // Create the global server log
    requests_log = create_log(); 
    pthread_mutex_init(&queue_mutex,NULL);
    pthread_cond_init(&master_condition,NULL);
    pthread_cond_init(&worker_condition,NULL);
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;
    int thread_num ;
    getargs(&port,&thread_num,&queue_size, argc, argv);
    waiting_tasks_queue = create();
    working_tasks_queue = create();
    pthread_array =(pthread_t*)malloc(sizeof(pthread_t)*thread_num);
    if(pthread_array == NULL){
        unix_error("Malloc Fail!");
        exit(1);
    }
    threads_stats* thread_stats_array = malloc(sizeof(struct Threads_stats)*thread_num);
    if(thread_stats_array == NULL){
        unix_error("Malloc Fail!");
        exit(1);
    }
    /* printf("---------------------------------thread num--------------------------------------------------%d****************************************************", thread_num); */
    for(int i = 0 ; i < thread_num ; i++){
        pthread_t thread;
        threads_stats stats = malloc(sizeof(struct Threads_stats));
        stats->id = i+1;             // Thread ID (placeholder)
        stats->stat_req = 0;       // Static request count
        stats->dynm_req = 0;       // Dynamic request count
        stats->post_req = 0 ;     //post request count
        stats->total_req = 0;      // Total request count
        thread_stats_array[i]=stats;
        pthread_create(&thread,NULL, &thread_function,(void*)&thread_stats_array[i]/*to be filled*/);//please notice that stats is the parameter of the thread function
        pthread_array[i]=thread;
    }
    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

        // TODO: HW3 — Record the request arrival time here

        // DEMO PURPOSE ONLY:
        // This is a dummy request handler that immediately processes
        // the request in the main thread without concurrency.
        // Replace this with logic to enqueue the connection and let
        // a worker thread process it from the queue.
        struct timeval arrival, dispatch;
        gettimeofday(&arrival,NULL);
        gettimeofday(&dispatch,NULL);
        //printf("lock in main \n");
        pthread_mutex_lock(&queue_mutex);
        //printf("lock in main success \n");
 // not the real dispatch time. this should be updated by thread worker when dequeued.

        while(getSize(working_tasks_queue)+getSize(waiting_tasks_queue)>=queue_size){
            //printf("entering cond_wait master_condition in main (beacuse of capacity)  \n");
            /* printf("main thread is going to wait, size of waiting queue is %d, size of working queue is %d, lastly, " */
                   /* "queue size : %d \n",getSize(waiting_tasks_queue), getSize(working_tasks_queue),queue_size); */
            pthread_cond_wait(&master_condition,&queue_mutex);
            //printf("entering cond_wait master_condition in main (beacuse of capacity), success \n");
        }
        //printf("main thread enqueues new task!\n");
        cntcnt++;
	
        enqueue(waiting_tasks_queue ,connfd, arrival,dispatch);
        //printf("main thread is going to wait, size of waiting queue is %d, size of working queue is %d, lastly, "
         //       "queue size : %d \n",getSize(waiting_tasks_queue), getSize(working_tasks_queue),queue_size);
        //printf("******************************************number of tasks so far is is "
           //    "%d****************************************\n",cntcnt);
        //printf("main thread successfully enqueued new task!\n");
        if (getSize(waiting_tasks_queue) > 0) {
            //printf("signaling  worker threads to work , after enqueueing the task in main  \n");
            pthread_cond_signal(&worker_condition);
            //printf("broadcasting  worker_condition in main success  \n");
        }
        //printf("mutex unlock  in main  \n");
        pthread_mutex_unlock(&queue_mutex);
        //printf("mutex unlock  in main success  \n");
    }

    // Clean up the server log before exiting
    destroy_log(requests_log);
    for(int i=0;i<thread_num;i++){
        free(thread_stats_array[i]); // Cleanup
        pthread_join(pthread_array[i],NULL);
    }
    free(thread_stats_array);
    free(pthread_array);
    pthread_mutex_destroy(&queue_mutex);
    pthread_cond_destroy(&master_condition);
    pthread_cond_destroy(&worker_condition);
    destroy(waiting_tasks_queue);
    destroy(working_tasks_queue);
    // TODO: HW3 — Add cleanup code for thread pool and queue
}
