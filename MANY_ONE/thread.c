#define _GNU_SOURCE
#include <unistd.h>
#include <sched.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <limits.h>
#include <ucontext.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <linux/futex.h>
#include <sys/time.h>
#include "Linked_List.h"
#include "scheduler.h"

#define STACK_SIZE (1024)

thread_control_block* curr_tcb = NULL;
thread_control_block* sched_tcb = NULL;
thread_control_block* main_tcb = NULL;
thread_LL threads_list;
sigset_t ignore_signals_list;
thread next_thread;

int* arr_exited_threads = NULL;
int count_exited_threads = 0;

void initialize_tcb(thread_control_block *t, int state, thread td, ucontext_t* context, void *routine, void *para){
    t->state_of_thread = state;
    t->tid = td;
    t->func = routine;
    t->args = para;    
    t->context = context;


    t->exited = 0;

    t->arr_waiting = NULL;
    t->count_of_waiters = 0;

    t->count_of_awaiting_Signals = 0;
    t->awaiting_signals = NULL;

    t->stack_beginning = NULL;
}

