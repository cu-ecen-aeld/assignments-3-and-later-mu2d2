#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>//for strerror

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    struct thread_data* args = (struct thread_data*)thread_param;
    if (!args)
    {
        return thread_param;
    }

    args->thread_complete_success = false;

    // Wait before attempting to obtain the mutex
    if (args->wait_to_obtain_ms > 0) 
    {
        (void)usleep(args->wait_to_obtain_ms * 1000u);
    }

    // Obtain mutex
    int rc = pthread_mutex_lock(args->mutex);
    if (rc != 0) 
    {
        ERROR_LOG("pthread_mutex_lock failed: %s", strerror(rc));
        return thread_param;
    }

    // Hold mutex for requested time
    if (args->wait_to_release_ms > 0) 
    {
        (void)usleep(args->wait_to_release_ms * 1000u);
    }

    // Release mutex
    rc = pthread_mutex_unlock(args->mutex);
    if (rc != 0) 
    {
        ERROR_LOG("pthread_mutex_unlock failed: %s", strerror(rc));
        return thread_param;
    }

    args->thread_complete_success = true;
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

    if (!thread || !mutex) 
    {
        ERROR_LOG("Invalid arguments (thread or mutex is NULL)");
        return false;
    }

    // Allocate and populate thread arguments
    struct thread_data *data = (struct thread_data*)malloc(sizeof(struct thread_data));
    if (!data) 
    {
        ERROR_LOG("malloc failed");
        return false;
    }

    data->mutex = mutex;
    data->wait_to_obtain_ms = wait_to_obtain_ms;
    data->wait_to_release_ms = wait_to_release_ms;
    data->thread_complete_success = false;

    int rc = pthread_create(thread, NULL, threadfunc, data);
    if (rc != 0) 
    {
        ERROR_LOG("pthread_create failed: %s", strerror(rc));
        free(data);
        return false;
    }

    return true;
}

