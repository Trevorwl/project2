#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

#define HZ 100

static struct sigaction old_sa;
static struct itimerval old_timer;
static bool preempt_enabled = false;
static sigset_t sigset;

static void alarm_handler(int signum)
{
    (void)signum;
    uthread_yield();
}

void preempt_start(bool preempt)
{
    if (!preempt)
        return;

    preempt_enabled = true;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGVTALRM);

    struct sigaction sa;
    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGVTALRM, &sa, &old_sa);

    struct itimerval timer;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 1000000 / HZ;
    timer.it_value = timer.it_interval;

    setitimer(ITIMER_VIRTUAL, &timer, &old_timer);
}

void preempt_stop(void)
{
    if (!preempt_enabled)
        return;

    setitimer(ITIMER_VIRTUAL, &old_timer, NULL);
    sigaction(SIGVTALRM, &old_sa, NULL);
}

void preempt_disable(void)
{
    if (preempt_enabled)
        sigprocmask(SIG_BLOCK, &sigset, NULL);
}

void preempt_enable(void)
{
    if (preempt_enabled)
        sigprocmask(SIG_UNBLOCK, &sigset, NULL);
}
