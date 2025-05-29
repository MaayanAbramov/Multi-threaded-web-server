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

// struct thread_worker{
//     pthread_t t;
//     t->id = 0;             // Thread ID (placeholder)
//     t->stat_req = 0;       // Static request count
//     t->dynm_req = 0;       // Dynamic request count
//     t->total_req = 0;      // Total request count

// };
//int cnt = 0;
pthread_t* pthread_array; 
// typedef struct thread_worker thread_info;
Queue waiting_tasks_queue, working_tasks_queue;
// Parses command-line arguments
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
        pthread_mutex_lock(&queue_mutex);
        while(!(getSize(waiting_tasks_queue)>0)) 
        {
            pthread_cond_signal(&master_condition);//doubts about this line SALEEM
            pthread_cond_wait(&worker_condition,&queue_mutex);
        }
        int fd = getHead(waiting_tasks_queue);
        struct timeval arrival_time = getArrivalTime(waiting_tasks_queue, fd);
        dequeue(waiting_tasks_queue);
        struct timeval dispatch_time;
        gettimeofday(&dispatch_time,NULL);
        enqueue(working_tasks_queue,fd,arrival_time,dispatch_time);
        pthread_mutex_unlock(&queue_mutex);

        requestHandle(fd, arrival_time, dispatch_time, t_stats, log);

        pthread_mutex_lock(&queue_mutex);
        removeQ(working_tasks_queue,fd);
        pthread_cond_signal(&master_condition);
        pthread_mutex_unlock(&queue_mutex);
        Close(fd);
        }
        return NULL;
}
int main(int argc, char *argv[])
{
    
    // Create the global server log
    server_log log = create_log();
    pthread_mutex_init(&queue_mutex,NULL);
    pthread_cond_init(&master_condition,NULL);
    pthread_cond_init(&worker_condition,NULL);
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;
    int queue_size, thread_num ;
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
    for(int i = 0 ; i < thread_num ; i++){
        pthread_t thread;
        threads_stats stats = malloc(sizeof(struct Threads_stats));
        stats->id = i;             // Thread ID (placeholder)
        stats->stat_req = 0;       // Static request count
        stats->dynm_req = 0;       // Dynamic request count
        stats->total_req = 0;      // Total request count
        pthread_create(&thread,NULL, &thread_function,(void*)&stats/*to be filled*/);//please notice that stats is the parameter of the thread function
        thread_stats_array[i]=stats;
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
        arrival.tv_sec = 0; arrival.tv_usec = 0;   // DEMO: dummy timestamps
        dispatch.tv_sec = 0; dispatch.tv_usec = 0; // DEMO: dummy timestamps
        pthread_mutex_lock(&queue_mutex);
        if(getSize(working_tasks_queue)+getSize(waiting_tasks_queue)>=queue_size){
            pthread_cond_wait(&master_condition,&queue_mutex);
        }
        enqueue(waiting_tasks_queue ,connfd, arrival,arrival);
        pthread_mutex_unlock(&queue_mutex);
        
    }

    // Clean up the server log before exiting
    destroy_log(log);
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
