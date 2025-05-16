#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#include "private.h"
#include "uthread.h"
#include "settings.h"

#define HZ 100

static struct sigaction old_sa;
static struct itimerval old_timer;
static bool preempt_enabled = false;
static sigset_t Sigset;

sigset_t old_mask;

extern int getTid(struct uthread_tcb* tcb);

static void alarm_handler(int signum)
{
    if(show_preempted_thread_debug==true){
        printf("thread %d preempted\n",getTid(uthread_current()));
    }

    (void)signum;
    uthread_yield();
}

void preempt_start(bool preempt)
{
    if (!preempt)
        return;

    sigprocmask(SIG_SETMASK, NULL, &old_mask);

    preempt_enabled = true;

    sigemptyset(&Sigset);
    sigaddset(&Sigset, SIGVTALRM);

    struct sigaction sa;
    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGVTALRM, &sa, &old_sa);

    struct itimerval timer;

    memset(&timer, 0, sizeof(timer));


    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 1000000 / HZ;
    timer.it_value = timer.it_interval;

    preempt_disable();

    setitimer(ITIMER_VIRTUAL, &timer, &old_timer);
}

void preempt_stop(void)
{
    if (!preempt_enabled)
        return;

    sigprocmask(SIG_SETMASK, &old_mask, NULL);

    setitimer(ITIMER_VIRTUAL, &old_timer, NULL);
    sigaction(SIGVTALRM, &old_sa, NULL);
}

void preempt_disable(void)
{
    if (preempt_enabled)
        sigprocmask(SIG_BLOCK, &Sigset, NULL);
}

void preempt_enable(void)
{
    if (preempt_enabled)
        sigprocmask(SIG_UNBLOCK, &Sigset, NULL);
}
