//#===================================
//#====   Created by rachelive   =====
//#===================================


//#======================
//#====   INCLUDES   ====
//#======================

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "threadpool.h"

//#======================
//#====   DEFINES   =====
//#======================

#define PERMITIONS 0
#define AINT_PERMITIONS 1

//#================================
//#====   create_threadpool   =====
//#================================

threadpool* create_threadpool(int num_threads_in_pool)
{

    if (num_threads_in_pool > MAXT_IN_POOL || num_threads_in_pool < 1)
    {
        printf("Error number of threads requested must to be smaller then MAXT_IN_POOL and bigger then num_threads_in_pool\n");
        return NULL;
    }


    threadpool *currectTreadpool = (threadpool*)calloc(1, sizeof(threadpool));
    if (!currectTreadpool)
    {
        perror("Malloc Error threadPool\n");
        return NULL;
    }


    currectTreadpool->num_threads = num_threads_in_pool;


    currectTreadpool->threads = (pthread_t*)calloc(currectTreadpool->num_threads, sizeof(pthread_t));
    if (!currectTreadpool->threads)
    {
        perror("Error malloc currectTreadpool\n");
        free(currectTreadpool);
        return NULL;
    }


    currectTreadpool->qhead = NULL;
    currectTreadpool->qtail = NULL;
    currectTreadpool->qsize = 0;


    currectTreadpool->shutdown = PERMITIONS;
    currectTreadpool->dont_accept = PERMITIONS;


    if (pthread_mutex_init(&(currectTreadpool->qlock), NULL))
    {
        perror("Error pthread_mutex_init\n");
        free(currectTreadpool->threads);
        free(currectTreadpool);
        return NULL;
    }
    if (pthread_cond_init(&(currectTreadpool->q_empty), NULL))
    {
        perror("Error pthread_cond_init\n");
        pthread_mutex_destroy(&(currectTreadpool->qlock));
        free(currectTreadpool->threads);
        free(currectTreadpool);
        return NULL;
    }
    if (pthread_cond_init(&(currectTreadpool->q_not_empty), NULL))
    {
        perror("pthread_cond_init Error\n");
        pthread_mutex_destroy(&(currectTreadpool->qlock));
        pthread_cond_destroy(&(currectTreadpool->q_empty));
        free(currectTreadpool->threads);
        free(currectTreadpool);
        return NULL;
    }


    int i;
    for (i = 0; i < currectTreadpool->num_threads; i++)
    {

        if (pthread_create(&(currectTreadpool->threads[i]), NULL, do_work, currectTreadpool))
        {
            perror("pthread_create Error\n");
            pthread_mutex_destroy(&(currectTreadpool->qlock));
            pthread_cond_destroy(&(currectTreadpool->q_empty));
            pthread_cond_destroy(&(currectTreadpool->q_not_empty));
            free(currectTreadpool->threads);
            free(currectTreadpool);
            return NULL;
        }
    }

    return currectTreadpool;
}


//#=======================
//#====   dispatch   =====
//#=======================

void dispatch(threadpool* from_me, dispatch_fn dispatch_to_here, void *arg)
{

    pthread_mutex_lock(&(from_me->qlock));

    if(from_me->dont_accept == AINT_PERMITIONS)
        return;

    pthread_mutex_unlock(&(from_me->qlock));

    if (!dispatch_to_here)
    {
        printf("Dispatch function not assigned correctly\n");
        destroy_threadpool(from_me);
    }


    work_t *newRequest = (work_t*)calloc(1, sizeof(work_t));
    if (!newRequest)
    {
        perror("malloc Error newRequest\n");
        return;
    }


    newRequest->routine = dispatch_to_here;
    newRequest->arg = arg;
    newRequest->next = NULL;

    pthread_mutex_lock(&(from_me->qlock));


    if(from_me->dont_accept == AINT_PERMITIONS)
    {
        free(newRequest);
        return;
    }

    if (!from_me->qsize)
    {
        from_me->qhead = newRequest;
        from_me->qtail = newRequest;
    }
    else
    {
        from_me->qtail->next = newRequest;
        from_me->qtail = from_me->qtail->next;
    }
    from_me->qsize++;

    pthread_cond_signal(&(from_me->q_not_empty));

    pthread_mutex_unlock(&(from_me->qlock));
}

//#=======================
//#====   do_work   ======
//#=======================

void* do_work(void* p)
{

    threadpool *myThreadPool = (threadpool*)p;
    while (1)
    {

        pthread_mutex_lock(&(myThreadPool->qlock));


        if(myThreadPool->shutdown)
        {

            pthread_mutex_unlock(&(myThreadPool->qlock));
            return NULL;
        }


        while (!myThreadPool->qsize)
        {

            pthread_cond_wait(&(myThreadPool->q_not_empty),&(myThreadPool->qlock));
            if (myThreadPool->shutdown)
            {

                pthread_mutex_unlock(&(myThreadPool->qlock));
                return NULL;
            }
        }

        myThreadPool->qsize--;
        work_t *temp = myThreadPool->qhead;
        if (!myThreadPool->qsize)
        {
            myThreadPool->qhead = NULL;
            myThreadPool->qtail = NULL;

            if (myThreadPool->dont_accept)
                pthread_cond_signal(&(myThreadPool->q_empty));
        }
        else
            myThreadPool->qhead = myThreadPool->qhead->next;

        pthread_mutex_unlock(&(myThreadPool->qlock));


        if (temp->routine(temp->arg) < 0)
            printf("Error routine\n");
        free(temp);
    }

}


//#==================================
//#====   destroy_threadpool   ======
//#==================================

void destroy_threadpool(threadpool* destroyme)
{

    pthread_mutex_lock(&(destroyme->qlock));

    destroyme->dont_accept = AINT_PERMITIONS;

    while (destroyme->qsize)
        pthread_cond_wait(&(destroyme->q_empty),&(destroyme->qlock));

    destroyme->shutdown = AINT_PERMITIONS;

    pthread_cond_broadcast(&(destroyme->q_not_empty));


    pthread_mutex_unlock(&(destroyme->qlock));


    int i; void *status;
    for (i = 0; i < destroyme->num_threads; i++)
        pthread_join(destroyme->threads[i], &status);


    pthread_mutex_destroy(&(destroyme->qlock));
    pthread_cond_destroy(&(destroyme->q_empty));
    pthread_cond_destroy(&(destroyme->q_not_empty));
    free(destroyme->threads);
    free(destroyme);
}

