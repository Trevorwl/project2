#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <ucontext.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"
#include "Constants.h"
#include "queue.h"
#include "settings.h"
#include "sem.h"

/*
 * Queue of waiting threads for scheduler
 */
queue_t threadQueue;

/*
 * Tcb of scheduling thread
 */
struct uthread_tcb* mainTcb = NULL;

/*
 * Tcb of thread dispatched by scheduler
 */
struct uthread_tcb* currentTcb = NULL;

/*
 * Set at initialization if user wants preemption.
 *
 * Used in preempt.c to prevent preemption
 * logic from being executed if preemptionAllowed=false
 */
bool preemptionAllowed = false;

/*
 * counter to set tids for uthread_tcb
 */
int tidCount = 0;

struct uthread_tcb {
    uthread_ctx_t context;
    void *stack;

    /*
     * Blocked, ready, running exited
     */
    int state;
    int tid;
};

void uthread_tcb_destructor(struct uthread_tcb* tcb){
    if(tcb->stack!=NULL){
        uthread_ctx_destroy_stack(tcb->stack);
        tcb->stack=NULL;
    }

    free(tcb);
}


int getTid(struct uthread_tcb* tcb){
    return tcb->tid;
}

struct uthread_tcb *uthread_current(void)
{
    return currentTcb;
}

void uthread_yield(void)
{
    /*
     * we stop interruptions before entering scheduler
     */
    preempt_disable();

    if(show_thread_yielding==true){
        printf("thread %d yields\n",currentTcb->tid);
    }

    currentTcb->state=READY;
    uthread_ctx_switch(&currentTcb->context, &mainTcb->context);

    /*
     * We enable interruptions for thread after it is woken up
     */
    preempt_enable();
}

void uthread_exit(void)
{
    preempt_disable();

	currentTcb->state=EXITED;
	uthread_ctx_switch(&currentTcb->context, &mainTcb->context);
}

int uthread_create(uthread_func_t func, void *arg)
{
    struct uthread_tcb* tcb=(struct uthread_tcb*)calloc(1,sizeof(struct uthread_tcb));
    if(!tcb){
        return -1;
    }

    tcb->stack=uthread_ctx_alloc_stack();

    if(!tcb->stack){
        uthread_tcb_destructor(tcb);
        return -1;
    }

    if(uthread_ctx_init(&tcb->context,tcb->stack,func,arg)==-1){

        uthread_tcb_destructor(tcb);
        return -1;
    }

    tcb->state=READY;
    tcb->tid=tidCount++;

    preempt_disable();
    queue_enqueue(threadQueue,tcb);
    preempt_enable();

    return 0;
}

/*
 * execution of this function always runs with preemption disabled.
 */
int uthread_run(bool preempt, uthread_func_t func, void *arg)
{

    threadQueue = queue_create();

    if(!threadQueue){
        return -1;
    }

    mainTcb=(struct uthread_tcb*)calloc(1,sizeof(struct uthread_tcb));

    if(!mainTcb){
        queue_destroy(threadQueue);
        threadQueue=NULL;

        return -1;
    }

    mainTcb->stack=NULL;
    mainTcb->state=RUNNING;

    if(getcontext(&mainTcb->context)){
        uthread_tcb_destructor(mainTcb);
        mainTcb=NULL;
        queue_destroy(threadQueue);
        threadQueue=NULL;
        return -1;
    }

    if(uthread_create(func,arg)){
        uthread_tcb_destructor(mainTcb);
        mainTcb=NULL;
        queue_destroy(threadQueue);
        threadQueue=NULL;
        return -1;
    }

    preemptionAllowed = preempt;

    preempt_start(preempt);

    while(queue_length(threadQueue)>0){

        /*
         * Get next ready thread
         */
        struct uthread_tcb* nextThread;

        queue_dequeue(threadQueue,(void**)&nextThread);

        currentTcb =  nextThread;

        if(show_thread_executing_debug==true){
            printf("thread %d executing on cpu\n",currentTcb->tid);
        }

        /*
         * Dispatch next thread
         */
        uthread_ctx_switch(&mainTcb->context, &currentTcb->context);


        /*
         * Check state of returning thread
         */
        if(currentTcb && currentTcb->state==EXITED){
            uthread_tcb_destructor(currentTcb);
            currentTcb=NULL;
        }

        if(currentTcb && currentTcb->state==READY){
            queue_enqueue(threadQueue, currentTcb);
        }
    }

    if(preempt){
        preempt_stop();
    }

    queue_destroy(threadQueue);
    threadQueue=NULL;
    uthread_tcb_destructor(mainTcb);
    mainTcb=NULL;

    return 0;
}

/*
 * It is recommended preemption be disabled when calling this.
 * It is left enabled to not interfere with users preemption logic,
 * but scheduler will be interrupted if preemption is left enabled.
 *
 * This function returns with preemption disabled.
 */
void uthread_block(void)
{
    if(show_threads_block_themselves==true){
        printf("thread %d blocks itself\n",currentTcb->tid);
    }

    currentTcb->state=BLOCKED;
    uthread_ctx_switch(&currentTcb->context,&mainTcb->context);
}

/*
 * It is recommended preemption be disabled when calling this.
 * It is left enabled to not interfere with users preemption logic,
 * but scheduler will be interrupted if preemption is left enabled.
 *
 *
 */
void uthread_unblock(struct uthread_tcb *uthread)
{
    if(show_threads_waking_others==true){
        printf("thread %d wakes up thread %d\n",currentTcb->tid,uthread->tid);
    }

    uthread->state=READY;
    queue_enqueue(threadQueue,uthread);
}



