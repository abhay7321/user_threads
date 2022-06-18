#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include "scheduler.h"
#include "Linked_List.h"

extern thread_control_block* sched_tcb;
extern thread_control_block* curr_tcb;
extern thread_LL threads_list;
extern int count_exited_threads;
extern int *arr_exited_threads;
extern sigset_t ignore_signals_list;


void timer_begin(){
    struct itimerval timer;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 3;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 3;
    setitimer(ITIMER_VIRTUAL, &timer, NULL);     
    return;
}

void timer_activate(){
    signal(SIGVTALRM, swtch);
    return;
}

void timer_deactivate(){
    signal(SIGVTALRM,SIG_IGN);
    return;
}

//For sigvtalarm function swtch is installed as the signal handler
//Hence ,  a sigvtalarm is generated after every x seconds , and on generation of that signal above function is invoked .
//It just saves the context of current running thread at swapcontext line and loads the context of scheduler thread .
//Every time we switch to scheduler context it always executes the sched function
//Also the sched function will always be executed from beginning and it will always execute completely because 
    // 1)We  used makecontext() in initializer that tells to run sched ,
    // 2)after that we never save its context at any arbitrary position as it happens with other threads
    // 3)it happens with other threads because of timer signal
    // 4)it never happens with scheduler because we disable tiimer once we enter it
void swtch(){
    swapcontext(curr_tcb->context, sched_tcb->context);
    timer_activate();
}

void sched(){

    timer_deactivate();

    change_state_shift(&threads_list);
    
    //To check if the thread that was executing currntly , has it exited
    int exit_thr_flag = 0;

    // Go through all the threads that are exited
    for(int i = 0; i < count_exited_threads; i++){
        thread_control_block* now = get_tcb_of_tid(&threads_list, arr_exited_threads[i]);

        // If current thread is exited then set the flag
        if(now->tid == curr_tcb->tid){
            exit_thr_flag = 1;
        }

        // All the threads that were waiting for the current thread change their state from WAITING to READY
        for(int j = 0; j < now->count_of_waiters; j++){
            thread_control_block* ready_tcb = get_tcb_of_tid(&threads_list, now->arr_waiting[j]);
            ready_tcb->state_of_thread = READY;
            shift_tail(&threads_list, ready_tcb);
        }

        // Delete the exited thread
        delete_thread(&threads_list, now->tid);
    }

    // free memory thats no more needed
    if(count_exited_threads){
        free(arr_exited_threads);
        arr_exited_threads = NULL;
    }

    // Setting its value to zero as all that were exited are deleted above and there are no more exited threads currently
    count_exited_threads = 0;

    populate_signal_set();

    thread_control_block *ahead = get_first_ready_thread(&threads_list);
    
    //return zala ithun tar kay honar , THINK on this..........
    if(!ahead) return;


    int k = ahead->count_of_awaiting_Signals;
    
    sigset_t mask;
    for(int i = 0 ;i < k;i++) {
        sigaddset(&mask,ahead->awaiting_signals[i]);
        sigprocmask(SIG_UNBLOCK,&mask,NULL);
        ahead->count_of_awaiting_Signals -= 1;
        raise(ahead->awaiting_signals[i]);
    }

    curr_tcb = ahead;
    ahead->state_of_thread = EXECUTING;

    if(setcontext(ahead->context) == -1){
        perror("Context Switch: ");
        exit(EXIT_FAILURE);
    }
}

void populate_signal_set(){
    sigfillset(&ignore_signals_list);
    sigdelset(&ignore_signals_list,SIGKILL);
    sigdelset(&ignore_signals_list,SIGSTOP);
    sigdelset(&ignore_signals_list,SIGCONT);
    sigdelset(&ignore_signals_list,SIGTERM);
    sigdelset(&ignore_signals_list,SIGVTALRM);
    sigprocmask(SIG_BLOCK,&ignore_signals_list,NULL);
}