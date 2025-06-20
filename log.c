#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "log.h"
#include "assert.h"

#ifndef NDEBUG
#      include <unistd.h>
#endif
int readers_inside, writers_inside, writers_waiting;
pthread_cond_t read_allowed;
pthread_cond_t write_allowed;
pthread_mutex_t global_lock;

void readers_writers_init() {
    readers_inside = 0;
    writers_inside = 0;
    writers_waiting = 0;
    pthread_cond_init(&read_allowed, NULL);
    pthread_cond_init(&write_allowed, NULL);
    pthread_mutex_init(&global_lock, NULL);
}
void readers_writers_destroy() {

    pthread_cond_destroy(&read_allowed);
    pthread_cond_destroy(&write_allowed);
    pthread_mutex_destroy(&global_lock);
}
void reader_lock() {
    pthread_mutex_lock(&global_lock);
    while (writers_inside > 0 || writers_waiting > 0)
        pthread_cond_wait(&read_allowed, &global_lock);
    readers_inside++;
    pthread_mutex_unlock(&global_lock);
}

void reader_unlock() {
    pthread_mutex_lock(&global_lock);
    readers_inside--;
    if (readers_inside == 0)
        pthread_cond_signal(&write_allowed);
    pthread_mutex_unlock(&global_lock);
}
void writer_lock() {
    pthread_mutex_lock(&global_lock);
    writers_waiting++;
    while (writers_inside + readers_inside > 0)
      pthread_cond_wait(&write_allowed, &global_lock);
    writers_waiting--;
    writers_inside++;
    pthread_mutex_unlock(&global_lock);
}

void writer_unlock() {
    pthread_mutex_lock(&global_lock);
    writers_inside--;
    if (writers_inside == 0) {
        pthread_cond_broadcast(&read_allowed);
        pthread_cond_signal(&write_allowed);
    }
    pthread_mutex_unlock(&global_lock);
}

// Opaque struct definition
struct Server_Log {
    char* log_buf;
    int len_log_buf;
    struct Server_Log* next;
    // TODO: Implement internal log storage (e.g., dynamic buffer, linked list, etc.)
};

// Creates a new server log instance (stub)
server_log create_log() {

    readers_writers_init();
    // TODO: Allocate and initialize internal log structure
    const char* dummy_buffer = "";
    int len = 0;
    server_log dummy = (server_log)malloc(sizeof(struct Server_Log));
    if(dummy == NULL){
        unix_error("Malloc error!\n");
        exit(1);
    }
    dummy->log_buf = (char*)malloc(sizeof(char)*1);
    if(dummy->log_buf == NULL){
        unix_error("Malloc Error \n");
        exit(1);
    }
    strcpy(dummy->log_buf, dummy_buffer);
    dummy->len_log_buf = len;
    dummy->next = NULL;
    return dummy;
}

// Destroys and frees the log (stub)
void destroy_log(server_log log) {
    server_log to_delete = NULL;
    server_log runner = log;
    while(runner!=NULL)
    {
        to_delete = runner;
        runner = runner->next;
        to_delete->next = NULL;
        free(to_delete->log_buf);
        to_delete->log_buf = NULL;
        free(to_delete);
    }

    readers_writers_destroy();
    /* free(log); */
    return;
}

// Returns dummy log content as string (stub)
int get_log(server_log log, char** dst) {
    reader_lock();
    // TODO: Return the full contents of the log as a dynamically allocated string
    // This function should handle concurrent access
    //strcat
    int total_len = 0;
    server_log tmp = log->next;//dont start from dummy.
    while(tmp!= NULL){
        assert(tmp->log_buf!= NULL && tmp->len_log_buf >= 0);
        total_len += tmp->len_log_buf;
        total_len += 1; // for a line delimiter '\n'
        tmp = tmp->next;
    }
    *dst = (char*)malloc(total_len + 1); // Allocate for caller
    int offset = 0 ;
    tmp = log->next;
    bool first_time = true;
    if (*dst != NULL) {
        while(tmp != NULL){
          if (!first_time) {
            // as explained in `https://piazza.com/class/m8nd0nnxsj77dt/post/377`
            // a new line should seperate
            (*dst)[offset] = '\n';
            offset += 1;
          }
          first_time = false;
          strncpy(*dst + offset,tmp->log_buf,tmp->len_log_buf);

          offset += tmp->len_log_buf;
          tmp = tmp->next;
        }
        (*dst)[offset] = '\0';
    }
    else{
        unix_error("Malloc Error!\n");
        exit(1);
    }
    reader_unlock();
    return offset;
}

// Appends a new entry to the log (no-op stub)
void add_to_log(server_log log, const char* data, int data_len) {
    // TODO: Append the provided data to the log
    // This function should handle concurrent access
    assert(strlen(data) == data_len);
    server_log to_insert = (server_log)malloc(sizeof(*to_insert));
    if(to_insert == NULL){
        unix_error("Malloc error");
        exit(1);
    }
    to_insert->log_buf = (char*)malloc(sizeof(char)*(data_len+1));
    if(to_insert->log_buf  == NULL){
        unix_error("Malloc error");
        exit(1);
    }
    strcpy(to_insert->log_buf, data);
    to_insert->len_log_buf = data_len;
    to_insert->next = NULL;

    writer_lock();
    server_log last = log;
    while(last->next != NULL) {
      last = last->next;
    }
    last->next = to_insert;

    writer_unlock();

}
