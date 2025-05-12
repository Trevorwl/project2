#include <stdio.h>
#include <stdbool.h>
#include <sem.h>
#include <stdlib.h>
#include<time.h>
#include <math.h>

#include "settings.h"
#include "uthread.h"

#define MAXCHAR 20

/*
 * This tester simulates a producer and consumer, where both threads
 * get timer interrupts throughout execution.
 *
 * Pass: consumer and producer never pass each other.
 *
 * Passing indicates that preemption is switching threads correctly,
 * and that semaphores are managing the producer and consumer correctly.
 *
 * Other tests should be performed, but this tests shows that the thread library works
 * for many cases involving preemption and semaphores.
 */

struct buffer{
    int writePos;
    int readPos;

    sem_t readerSem;
    sem_t writerSem;
};

void busy_wait(){
    volatile unsigned long long val = 0;

    (void)val;
      for (unsigned long long i = 0; i < 10000000ULL; ++i) {
          val += i;
     }
}

void reader(void* arg){
    struct buffer* b = (struct buffer*)arg;

    for(int j = 0; j < 30; j++){
        sem_down(b->readerSem);
        sem_up(b->writerSem);

        printf("reading at pos %d\n",b->readPos);
        b->readPos = (b->readPos + 1) % MAXCHAR;

        if(rand() % 10 == 0){
            busy_wait();
        }
    }
}

void writer(void* arg){
    struct buffer* b=(struct buffer*)arg;

    for(int j = 0; j < 30; j++){
        sem_down(b->writerSem);
        printf("writing at pos %d\n",b->writePos);

        b->writePos=(b->writePos+1) % MAXCHAR;

        sem_up(b->readerSem);

        if(rand() % 10 == 0){
            busy_wait();
        }
    }
}

void init(void* arg){
    uthread_create(reader,arg);
    uthread_create(writer,arg);
}

int main(){
    struct buffer b;
    srand(time(0));

    b.readPos=0;
    b.writePos=0;

    b.writerSem=sem_create(MAXCHAR);
    b.readerSem=sem_create(0);

    show_preempted_thread_debug=true;

    uthread_run(true,init,&b);
}
