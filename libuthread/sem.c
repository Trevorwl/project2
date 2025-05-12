#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "private.h"
#include "sem.h"
#include "uthread.h"
#include "Constants.h"

struct semaphore {
    /*
     * Resource count
     */
	int count;

	/*
	 * Waiting queue for threads blocked on
	 * semaphore. Waiting threads go here instead
	 * of scheduler, and are put on sheduler after
	 * waking up.
	 */
	queue_t blockedThreads;
};

sem_t sem_create(size_t count)
{
    sem_t sem=(sem_t)calloc(1,sizeof(struct semaphore));

    if(!sem){
        return NULL;
    }

    sem->count=count;

    sem->blockedThreads=queue_create();

    if(!sem->blockedThreads){
        free(sem);
        return NULL;
    }

    return sem;
}

int sem_destroy(sem_t sem)
{
    preempt_disable();

    if(sem==NULL ||queue_length(sem->blockedThreads) > 0){
        preempt_enable();
        return -1;
    }

    queue_destroy(sem->blockedThreads);
    sem->blockedThreads=NULL;
    free(sem);

    preempt_enable();

    return 0;
}

int sem_down(sem_t sem)
{
    if(sem==NULL){
        return -1;
    }

    /*
     * Disable preemption
     * so that this thread doesnt get interrupted,
     * and other threads inappropriately access sem->count
     * and queue_enque
     */
    preempt_disable();

    /*
     * if 0, go on semaphores queue and wait for a thread
     * to call sem_up to wake it.
     */
    if(sem->count==0){

        queue_enqueue(sem->blockedThreads,uthread_current());

        uthread_block();


    } else{

        //thread is allowed into critical section, decrement availability
        (sem->count)--;

    }

    preempt_enable();

    return 0;
}

int sem_up(sem_t sem)
{
    if(sem==NULL){
        return -1;
    }


    /*
     * Disable preemption
     * so that this thread doesnt get interrupted,
     * and other threads inappropriately access sem->blockedThreads
     * and queue functions
     */
    preempt_disable();

    //wake next thread, dont increment availability(we want this thread to have sem first)
    if(queue_length(sem->blockedThreads)>0){
        struct uthread_tcb* wakingThread;

        queue_dequeue(sem->blockedThreads,(void**)&wakingThread);


        uthread_unblock(wakingThread);

    } else {

        //no threads to wake, increment availability
        (sem->count)++;
    }

    preempt_enable();

    return 0;
}

