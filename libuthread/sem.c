#include <stddef.h>
#include <stdlib.h>
#include "queue.h"
#include "private.h"
#include "sem.h"

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
    preempt_disable();
    if (!sem || queue_length(sem->wait_queue) > 0){

        preempt_enable();
        return -1;
    }

    queue_destroy(sem->wait_queue);
    free(sem);
    preempt_enable();
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

        uthread_block();

    } else {

        (sem->count)--;
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
