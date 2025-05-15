#include <stddef.h>
#include <stdlib.h>
#include "queue.h"
#include "private.h"
#include "sem.h"


///////removed 4 lines, added 2 to have compatibility with uthread.c

struct semaphore {
    size_t count;
    queue_t wait_queue;
};


sem_t sem_create(size_t count)
{
    sem_t sem = calloc(1,sizeof(struct semaphore));
    if (!sem)
        return NULL;

    sem->count = count;
    sem->wait_queue = queue_create();
    if (!sem->wait_queue) {
        free(sem);
        return NULL;
    }

    return sem;
}

int sem_destroy(sem_t sem)
{
    preempt_disable();/////////////////////////////////maybe we add:
    if (!sem || queue_length(sem->wait_queue) > 0){  //ensure sem's queue is accessed uninterrupted

        preempt_enable();/////////////////////////////added
        return -1;
    }

    queue_destroy(sem->wait_queue);
    free(sem);
    preempt_enable();/////////////////////////////////added
    return 0;
}

int sem_down(sem_t sem)
{
    if (!sem)
        return -1;

    preempt_disable();
    if (sem->count == 0) {
        struct uthread_tcb *self = uthread_current();
        queue_enqueue(sem->wait_queue, self);
//        preempt_enable();   ///////////////////////////////////maybe we remove:
        uthread_block();                                      // i thought maybe be we leave preempt disabled
                                                              // to reenter scheduler on 1 disable call
//        // After unblocking, recheck availability
//        return sem_down(sem);  /////////////////////////////////maybe we remove:
                                                                //in sem_up i noticed the code doesnt
                                                                //increment sem->count after unblock,
                                                                //which leaves sem->count=0. This means
                                                                //other threads will block and this
                                                                //thread has the sem.
                                                                //
                                                                //I think its actually good we
                                                                //keep sem->count=0 because
                                                                //we guarantee this thread
                                                                //doesnt starve.


    } else { //////////////////////////////////////maybe we add:
                                                 //if we are unblocked, we might leave sem->count=0
                                                 //to block other threads.
        (sem->count)--;                          //if we never blocked, we decrement sem->count.
    }

    preempt_enable();
    return 0;
}

int sem_up(sem_t sem)
{
    if (!sem)
        return -1;

    preempt_disable();
    struct uthread_tcb *next;
    if (queue_dequeue(sem->wait_queue, (void**)&next) == 0) {
        uthread_unblock(next);
    } else {
        sem->count++;
    }
    preempt_enable();
    return 0;
}
