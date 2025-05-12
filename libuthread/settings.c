#include "settings.h"
#include "uthread.h"
#include "private.h"
#include "sem.h"

/*
 * Constants set by user to enable error messages
 */
bool show_thread_executing_debug = false;

bool show_preempted_thread_debug = false;

bool show_threads_block_themselves = false;

bool show_threads_waking_others = false;

bool show_thread_yielding = false;

void enableAllMessages(){
    show_thread_executing_debug = true;
    show_preempted_thread_debug = true;

    show_threads_block_themselves = true;

    show_threads_waking_others = true;

    show_thread_yielding = true;
}
