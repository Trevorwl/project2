#include <stdio.h>
#include <stdbool.h>
#include <sem.h>

#include "settings.h"
#include "uthread.h"


/* This test uses several threads using a semaphore with 2 resources. Active threads
 * are preempted throughout execution.
 *
 * pass: should see 2 threads at a time interleaving in output, while other threads are blocked.
 * 
 * passing indicates that threads are blocking and being woken up correctly, and that
 * preempted threads are being scheduled properly.
 */
void long_operation(void *arg) {
    (void)arg;

    sem_t mutex=(sem_t)arg;

    sem_down(mutex);

    volatile unsigned long long val = 0;
    for (unsigned long long i = 0; i < 100000000ULL; ++i) {
        val += i;
    }

    printf("Long task completed: %llu\n", val);

    sem_up(mutex);

}

void mainTask(void*arg){
    for(int i=0;i<4;i++){
        uthread_create(long_operation,arg);
    }
}

int main(){

    sem_t mutex=sem_create(2);

    show_thread_executing_debug=true;
    show_preempted_thread_debug=true;

    show_threads_block_themselves=true;
    show_threads_waking_others=true;

    uthread_run(true,mainTask,mutex);
}
