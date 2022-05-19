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



// TASK1)Execute the function that the thread was job of thread 
// TASK2)ONCE function is executed job of thread is done , increment exited count and add the tid of thread to that exited array
void start_routine(void *tcb){
    populate_signal_set();

    thread_control_block *tcb1 = (thread_control_block *)tcb;    
    
    //execute the function that was the job of thread
    timer_activate();
    tcb1->result = tcb1->func(tcb1->args);
    timer_deactivate();

    // when done with execution of thread , increase the count_exited_threads by 1.
    // also add the tid of current thread to arr_exited_threads
    arr_exited_threads = (int*)realloc(arr_exited_threads, (++count_exited_threads)*sizeof(int));
    arr_exited_threads[count_exited_threads-1] = curr_tcb->tid;


    timer_activate();

    swtch();
    //Once done with job of thread invoke swtch()
    // swtch() will invoke scheduler so that other thread can be scheduled
}


//TASK1)IF its first thread , then give call to intialize_lib() to do initialization stuffs .
//TASK2)Creates a new thread to run the function passed as argument and initializes that thread along with its context
//TASK3)VERY IMPORTANTLY , it uses makecontext() function to run the start routine function when the above thread is scheduler for the first time .
//TASK4)Pushes the thread in Linked_LIST
//TASK5)Activates the timer .
int thread_create(thread *td, void *start_func, void *para){
    timer_deactivate();

    // Library must be initialized only once .
    // So instead of asking user to initialize the library ,we are initializing it ,
    // while creating the first thread
    static int isInit = 0;
    if(!isInit){
        intialize_lib();
        isInit = 1;
    }


    thread_control_block *temp = (thread_control_block *)malloc(sizeof(thread_control_block));

    ucontext_t *thread_context = (ucontext_t *)malloc(sizeof(ucontext_t));
    getcontext(thread_context);

    temp->stack_beginning = (void *)malloc(sizeof(char)*STACK_SIZE);

    thread_context->uc_stack.ss_sp = temp->stack_beginning;
    thread_context->uc_stack.ss_size = STACK_SIZE;
    thread_context->uc_link = main_tcb->context;

    initialize_tcb(temp, READY, next_thread++, thread_context, start_func, para);

    makecontext(thread_context,(void*)&start_routine,1,(void *)(temp));

    push_thread(&threads_list,temp);
    *td = temp->tid;

    timer_activate();
    return 0;
}


int thread_exit(void *retVal){
    timer_deactivate();
    curr_tcb->result = retVal;
    arr_exited_threads = (int*)realloc(arr_exited_threads, (++count_exited_threads)*sizeof(int));
    arr_exited_threads[count_exited_threads-1] = curr_tcb->tid;
    swtch(); 
}



int thread_kill(thread td, int signal){
    timer_deactivate();

    if (signal < 1 || signal > 64){
        printf("Invalid signal\n");
        return EINVAL;
    }

    if(signal == SIGINT || signal == SIGCONT || signal == SIGSTOP|| signal == SIGKILL){
        kill(getpid(),signal);
    }
    else{
        if(curr_tcb->tid == td){
            raise(signal);
            timer_activate();
            return 0;
        }

        thread_control_block *now = get_tcb_of_tid(&threads_list,td);

        if (now == NULL){
            printf("Target thread does not exist \n");
            return ESRCH;
        }

        now->awaiting_signals = (int *)realloc(now->awaiting_signals, (++(now->count_of_awaiting_Signals) * sizeof(int)));
        now->awaiting_signals[now->count_of_awaiting_Signals - 1] = signal;
    }
    timer_activate();
    return 0;
}