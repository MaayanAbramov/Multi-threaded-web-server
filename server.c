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
struct thread_worker{
    pthread_t t;
};
int cnt = 0; 
typedef struct thread_worker thread_info;
Queue q;
// Parses command-line arguments
void getargs(int *port, int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
}
// TODO: HW3 — Initialize thread pool and request queue
// This server currently handles all requests in the main thread.
// You must implement a thread pool (fixed number of worker threads)
// that process requests from a synchronized queue.
void* thread_function(void* arg){
    if (getSize(q) != 0) {
        Queue head = getHead(q);
        requestHandle(head, getArrivalTime(q,head ), )
    }
 return NULL;
}
int main(int argc, char *argv[])
{
    printf("wtf\n");
    // Create the global server log
    server_log log = create_log();
    
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;
    int thread_num=8; // this shouldn't be 8, just a temp
    getargs(&port, argc, argv);
    printf("what the fuck\n");
    q = create();
    thread_info* thread_array = (thread_info*)malloc(sizeof(thread_info)*thread_num);
    if(thread_array == NULL){
        unix_error("Malloc Fail!");
        exit(1);
    }
    for(int i=0;i<thread_num;i++){
        pthread_t t1;
        pthread_create(&t1,NULL, &thread_function,(void*)NULL/*to be filled*/);
        thread_info t_info ;
        t_info.t=t1;
        thread_array[i]=t_info;
    }
    printf("ok\n");
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

        threads_stats t = malloc(sizeof(struct Threads_stats));
        t->id = 0;             // Thread ID (placeholder)
        t->stat_req = 0;       // Static request count
        t->dynm_req = 0;       // Dynamic request count
        t->total_req = 0;      // Total request count

        struct timeval arrival, dispatch;
        arrival.tv_sec = 0; arrival.tv_usec = 0;   // DEMO: dummy timestamps
        dispatch.tv_sec = 0; dispatch.tv_usec = 0; // DEMO: dummy timestamps
        // gettimeofday(&arrival, NULL);
        enqueue(q,connfd, arrival,dispatch);
        // Call the request handler (immediate in main thread — DEMO ONLY)
        requestHandle(connfd, arrival, dispatch, t, log);
        dequeue(q);
        free(t); // Cleanup
        Close(connfd); // Close the connection
    }

    // Clean up the server log before exiting
    destroy_log(log);
    for(int i=0;i<thread_num;i++){
        pthread_join(thread_array[i].t,NULL);
    }
    free(thread_array);
    // TODO: HW3 — Add cleanup code for thread pool and queue
}
