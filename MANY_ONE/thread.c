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



// Task 1)Initilaizes main thread along with its context
// Task 2)Initilaizes scheduler thread along with its context ,and uses makecontext to invoke sched function whenever setcontext() or swtchcontext() is done on scheduler ke thread ka context
// Thats all that we have done in initialization
void intialize_lib(){
    populate_signal_set();

    //Initializing thread Linked List
    threads_list.tail = NULL;
    threads_list.head = NULL;
    
    //Allocating space for tcb of main thread
    main_tcb = (thread_control_block*)malloc(sizeof(thread_control_block));
    
    //Initializing context of main thread
    ucontext_t* thread_context = (ucontext_t *)malloc(sizeof(ucontext_t));
    getcontext(thread_context);

    //Initializing tcb of main thread
    initialize_tcb(main_tcb, EXECUTING, getpid(), thread_context, NULL, NULL);
    
    //As its the current executing thread , hence setting current thread as the main thread
    curr_tcb = main_tcb;

    //We are not using clone in many one as we had used it in one-one
    //So giving thread_id is our task..........
    next_thread = getpid()+1;
    
    //Pusing main thread in Linked_List
    push_thread(&threads_list,main_tcb);

    //Allocating space for tcb of scheduler thread    
    sched_tcb = (thread_control_block*)malloc(sizeof(thread_control_block));
    
    //Initializing context of scheduler thread
    thread_context = (ucontext_t *)malloc(sizeof(ucontext_t));    
    getcontext(thread_context);

    thread_context->uc_stack.ss_sp = (void *)malloc(sizeof(char)*STACK_SIZE);
    thread_context->uc_stack.ss_size = STACK_SIZE;
    thread_context->uc_link = main_tcb->context;
    
    makecontext(thread_context, sched, 0);

    initialize_tcb(sched_tcb, EXECUTING, 0, thread_context, NULL, NULL);
    
    timer_begin();
}

