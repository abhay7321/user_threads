#define _GNU_SOURCE
#include<sched.h>
#include<setjmp.h>
#define CLONE_FLAGS CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SYSVSEM | CLONE_SIGHAND | CLONE_THREAD | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID

enum join_status {DETACHED, JOINABLE, JOINED};

typedef struct thread_control_block{
    int tid;
    void *stack_beginning;
    int stack_size;
    void *args;
    void *result;
    void *(*func)(void *);
    unsigned int futex_val;
    jmp_buf buf;
    int status_of_join;
    int joined_on;
}thread_control_block;


typedef unsigned long thread;

void clean_thread_resources(thread_control_block *tcb);

int start_routine(void *);
int thread_create(thread *, void *(*f)(void *), void *, int join_status);
int thread_join(thread, void **);
int send_signal_all(int signum);
int thread_kill(thread tcb, int signum);
void thread_exit(void *retval);