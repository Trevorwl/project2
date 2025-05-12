#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "private.h"
#include "uthread.h"
#include "settings.h"

/*
 * Frequency of preemption
 */
#define HZ 100

/*
 * Holds the action for SIGVTALRM before
 * scheduler is run. This is restored when
 * schduler is done (preempt_stop())
 */
struct sigaction oldTimerAction;

/*
 * Timer interval before scheduler was run.
 */
struct itimerval oldTimerInterval;

/*
 * Signal mask of process before scheduler was run.
 */
sigset_t oldMask;

/*
 * Set to true by scheduler if user wants preemption, false otherwise.
 *
 * Prevents preemption logic from being executed if preemptionAllowed=false.
 */
extern bool preemptionAllowed;

extern int getTid(struct uthread_tcb* tcb);

/*
 * This is called when dispatched thread is interrupted.
 *
 * We yield back to the scheduler.
 *
 */
void timerHandler(int signal){

    if(show_preempted_thread_debug==true){
        printf("thread %d preempted\n",getTid(uthread_current()));
    }

    uthread_yield();
}

/*
* Unblock SIGVTARLM to allow threads to be interrupted
* by kernel, which is has been constantly sending SIGVTALRM.
*/
void preempt_disable(void)
{
    if(preemptionAllowed==false){
        return;
    }

    sigset_t blockAlarmMask;

    sigemptyset(&blockAlarmMask);
    sigaddset(&blockAlarmMask, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &blockAlarmMask, NULL);
}

/*
* Block SIGVTARLM to prevent threads from being interrupted by kernel
* by kernel, which will continue to send SIGVTALRMM.
*/
void preempt_enable(void)
{
    if(preemptionAllowed==false){
        return;
    }

    sigset_t unblockAlarmMask;

    sigemptyset(&unblockAlarmMask);
    sigaddset(&unblockAlarmMask, SIGVTALRM);
    sigprocmask(SIG_UNBLOCK, &unblockAlarmMask, NULL);
}


void preempt_start(bool preempt)
{
    if(preempt==false){
        return;
    }

    /*
     * Save old process signal mask
     */
    sigprocmask(SIG_SETMASK, NULL, &oldMask);

    struct sigaction newTimerAction;


    newTimerAction.sa_handler=timerHandler;
    sigemptyset(&newTimerAction.sa_mask);

    /*
     * Flag is set for timer action so that
     * i/o syscalls are restarted if thread is interrupted by timer.
     */
    newTimerAction.sa_flags=SA_RESTART;

    /*
     * Save old sigaction and set new one
     */
    sigaction(SIGVTALRM,&newTimerAction,&oldTimerAction);

    struct itimerval newTimerInterval;

    memset(&newTimerInterval, 0, sizeof(newTimerInterval));

    /*
     * Set delay for timer
     */
    newTimerInterval.it_value.tv_sec = 0;
    newTimerInterval.it_value.tv_usec = 1000000 / HZ;

    /*
     * Set interval for timer
     */
    newTimerInterval.it_interval.tv_sec = 0;
    newTimerInterval.it_interval.tv_usec = 1000000 / HZ;

    /*
     * disable preemption so scheduler can run
     */
    preempt_disable();


    /*
     * Set new timer, save old one
     */
    setitimer(ITIMER_VIRTUAL, &newTimerInterval, &oldTimerInterval);
}

void preempt_stop(void)
{
    if(preemptionAllowed==false){
        return;
    }

    /*
     * Restore previous process mask
     */
    sigprocmask(SIG_SETMASK, &oldMask, NULL);

    /*
     * Restore old timer action
     */
    sigaction(SIGVTALRM,&oldTimerAction,NULL);

    /*
     * Restore old timer
     */
    setitimer(ITIMER_VIRTUAL, &oldTimerInterval, NULL);
}

