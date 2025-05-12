
#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <stdbool.h>

extern bool show_thread_executing_debug;

extern bool show_preempted_thread_debug;

extern bool show_threads_block_themselves;

extern bool show_threads_waking_others;

extern bool show_thread_yielding;

void enableAllMessages();


#endif
