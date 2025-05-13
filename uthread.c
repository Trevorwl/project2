#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

enum thread_state {
    RUNNING,
    READY,
    BLOCKED,
    EXITED
};

struct uthread_tcb {
    ucontext_t context;
    void *stack;
    enum thread_state state;
};

static queue_t ready_queue;
static struct uthread_tcb *current_thread = NULL;
static struct uthread_tcb *idle_thread = NULL;
static bool preempt_enabled = false;

#define STACK_SIZE 32768

struct uthread_tcb *uthread_current(void)
{
    return current_thread;
}

void uthread_yield(void)
{
    preempt_disable();
    struct uthread_tcb *prev = current_thread;
    struct uthread_tcb *next;

    if (queue_dequeue(ready_queue, (void**)&next) == 0) {
        if (prev->state == RUNNING) {
            prev->state = READY;
            queue_enqueue(ready_queue, prev);
        }
        current_thread = next;
        next->state = RUNNING;
        uthread_ctx_switch(&prev->context, &next->context);
    }
    preempt_enable();
}

void uthread_exit(void)
{
    preempt_disable();
    current_thread->state = EXITED;
    free(current_thread->stack);
    free(current_thread);

    if (queue_length(ready_queue) == 0) {
        setcontext(&idle_thread->context);
    } else {
        struct uthread_tcb *next;
        queue_dequeue(ready_queue, (void**)&next);
        current_thread = next;
        next->state = RUNNING;
        setcontext(&next->context);
    }
}

int uthread_create(uthread_func_t func, void *arg)
{
    struct uthread_tcb *tcb = malloc(sizeof(struct uthread_tcb));
    if (!tcb)
        return -1;

    tcb->stack = uthread_ctx_alloc_stack();
    if (!tcb->stack) {
        free(tcb);
        return -1;
    }

    if (uthread_ctx_init(&tcb->context, tcb->stack, func, arg) == -1) {
        uthread_ctx_destroy_stack(tcb->stack);
        free(tcb);
        return -1;
    }

    preempt_disable();
    tcb->state = READY;
    queue_enqueue(ready_queue, tcb);
    preempt_enable();
    return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
    preempt_enabled = preempt;
    if (preempt_enabled)
        preempt_start(preempt);

    ready_queue = queue_create();
    if (!ready_queue)
        return -1;

    idle_thread = malloc(sizeof(struct uthread_tcb));
    if (!idle_thread)
        return -1;

    current_thread = idle_thread;
    idle_thread->state = RUNNING;

    if (uthread_create(func, arg) == -1)
        return -1;

    while (queue_length(ready_queue) > 0) {
        uthread_yield();
    }

    free(idle_thread);

    if (preempt_enabled)
        preempt_stop();

    return 0;
}

void uthread_block(void)
{
    preempt_disable();
    struct uthread_tcb *prev = current_thread;
    prev->state = BLOCKED;

    struct uthread_tcb *next;
    if (queue_dequeue(ready_queue, (void**)&next) == 0) {
        current_thread = next;
        next->state = RUNNING;
        uthread_ctx_switch(&prev->context, &next->context);
    } else {
        setcontext(&idle_thread->context);
    }
    preempt_enable();
}

void uthread_unblock(struct uthread_tcb *uthread)
{
    preempt_disable();
    if (uthread->state == BLOCKED) {
        uthread->state = READY;
        queue_enqueue(ready_queue, uthread);
    }
    preempt_enable();
}
