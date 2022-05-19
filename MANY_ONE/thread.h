#define _GNU_SOURCE
#include<stdatomic.h>
#include<unistd.h>
#include<ucontext.h>

enum all_states {READY, EXECUTING, WAITING, FINISHED};

typedef unsigned long int thread;

typedef struct thread_control_block {
    thread tid;

    void *stack_beginning;
    int stack_size;

    void *args;
    void *(*func)(void *);

    int state_of_thread;
    ucontext_t *context;

    void *result;
    int exited;

    int *awaiting_signals;
    int count_of_awaiting_Signals;

    int *arr_waiting;
    int count_of_waiters;

} thread_control_block;


void initialize_tcb(thread_control_block *, int, thread, ucontext_t*, void *routine, void *arg);
void init_lib();
void start_routine(void *);
int thread_create(thread *, void *, void *);


int thread_join(thread , void **);
int thread_kill(thread, int);
int thread_exit(void *);