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
