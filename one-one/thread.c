#include "Linked_List.h"
#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <limits.h>
#include <setjmp.h>
#include <errno.h>
#include "lock.h"

thread_LL *threads_list;

#define STACK_SIZE (1024)
#define TGKILL 234


int start_routine(void *tcb){
    thread_control_block *tcb1 = (thread_control_block *)tcb;

    if (setjmp(tcb1->buf) == 0){
        void *result = tcb1->func(tcb1->args);

        // Implicitly calling thread_exit.
        // If the user routine doesn't call thread_exit then also give call to it.
        thread_exit(result);
    }

    if (tcb1->status_of_join == DETACHED){
        // free the thread resources here only.
        clean_thread_resources(tcb);
    }

    // this is the exit status for the current thread.
    return 0;
}

int init_lib()
{

    mutexlock_init(&LL_lock);
    mutexlock_init(&initializer_lock);

    threads_list = (thread_LL *)malloc(sizeof(thread_LL));

    if (!threads_list)
    {
        return -1;
    }

    threads_list->head = NULL;
    threads_list->tail = NULL;

    return 0;
}

int thread_create(thread *t, void *(*f)(void *), void *args, int join_status)
{

    static int initializer = 0;

    thread_mutex_lock(&initializer_lock);

    if (initializer == 0)
    {

        int r = init_lib();

        if (r != 0)
        {

            printf("Library Initializaton error due to memory allocation\n");
            thread_mutex_unlock(&initializer_lock);
            return EAGAIN;
        }

        initializer = 1;
    }

    thread_mutex_unlock(&initializer_lock);

    if (!t)
    {
        printf("Invalid thread ID\n");
        return EINVAL;
    }

    if (!f)
    {
        printf("Invalid rountine function\n");
        return EINVAL;
    }

    if (!(join_status == DETACHED || join_status == JOINABLE || join_status == JOINED))
    {

        printf("Invalid Join Status\n");
        return EINVAL;
    }

    thread_control_block *tcb = (thread_control_block *)malloc(sizeof(thread_control_block));

    if (tcb == NULL)
    {
        printf("Thread allocation error due to memory allocation \n");
        return EAGAIN;
    }

    char *stack;
    stack = (char *)malloc(STACK_SIZE);

    if (stack == NULL)
    {
        printf("Stack allocation error due to memory allocation\n");
        return EAGAIN;
    }

    tcb->stack_beginning = stack + STACK_SIZE - 1;
    tcb->args = args;
    tcb->func = f;
    tcb->stack_size = STACK_SIZE;
    tcb->status_of_join = join_status;

    int r = LL_push(threads_list, tcb);

    if (r != 0)
    {

        printf("New thread is not pushed in linked list\n");
        free(tcb);
        return EAGAIN;
    }

    tcb->tid = clone(start_routine, tcb->stack_beginning, CLONE_FLAGS, tcb, &tcb->futex_val, tcb, &tcb->futex_val);

    if (tcb->tid == -1)
    {

        printf("Clone error\n");
        return errno;
    }

    *t = tcb->tid;
    return 0;
}

void clean_thread_resources(thread_control_block *tcb)
{

    node_LL *n = get_node(threads_list, tcb->tid);

    LL_deletenode(threads_list, n);

    // free stack
    free((void *)(tcb->stack_beginning + 1 - STACK_SIZE));

    // free thread
    free(tcb);

    // free linked list node for the thread.
    free(n);

    return;
}

int thread_join(thread t, void **result){
    thread_control_block *tcb = get_info_tcb(threads_list, t);
    thread_control_block *curr = get_info_tcb(threads_list, gettid());

    // If no such thread exists.
    if (tcb == NULL){
        printf("No thread with the id thread could be found\n");
        return ESRCH;
    }

    if (tcb->status_of_join != JOINABLE){
        if (tcb->status_of_join == DETACHED){
            printf("Target thread is detached thread\n");
            return EINVAL;
        }

        else if (tcb->status_of_join == JOINED){
            printf("Target thread is already is already joined\n");
            return EINVAL;
        }
    }

    // if thread A is waiting for thread B and then B calls join i.e. wants to wait for A then deadlock happens.
    if (curr != NULL && curr->joined_on == tcb->tid){
        printf("A Deadlock is detected \n");
        return EDEADLK;
    }

    tcb->joined_on = gettid();
    tcb->status_of_join = JOINED;

    // If target thread's execution isn't complete then wait for thread to complete its function execution
    if ((tcb->futex_val == tcb->tid)){
        syscall(SYS_futex, &tcb->futex_val, FUTEX_WAIT, tcb->tid, NULL, NULL, 0);
    }

    if (result != NULL){
        *result = tcb->result;
    }

    clean_thread_resources(tcb);

    return 0;
}

int send_signal_all(int signum){
    thread_mutex_lock(&LL_lock);

    node_LL *temp = threads_list->head;
    int pid = getpid();
    int ret_val;

    while (temp){
        if ((temp->tcb)->tid == gettid()){
            temp = temp->next;
            continue;
        }

        ret_val = syscall(TGKILL, pid, (temp->tcb)->tid, signum);

        if (signum == SIGKILL || signum == SIGTERM){
            thread_mutex_unlock(&LL_lock);
            LL_deletenode(threads_list, temp);
            thread_mutex_lock(&LL_lock);
        }
        temp = temp->next;
    }

    thread_mutex_unlock(&LL_lock);
    return 0;
}

// For SIGCONT, SIGSTOP, SIGKILL, SIGTERM signals : send signal to all the threads
// and for other signals send the signal to target thread.
int thread_kill(thread t, int signum)
{

    if (signum < 1 || signum > 64){
        printf("Invalid signal\n");
        return EINVAL;
    }

    thread_control_block *tcb = get_info_tcb(threads_list, t);

    if (tcb == NULL){
        printf("Target thread does not exist \n");
        return ESRCH;
    }

    int ret_val;

    if (signum == SIGTERM || signum == SIGKILL || signum == SIGCONT || signum == SIGSTOP){
        send_signal_all(signum);
        int pid = getpid();

        ret_val = syscall(TGKILL, pid, gettid(), signum);

        if (ret_val == -1)
            return errno;

        return 0;
    }

    int pid = getpid();
    ret_val = syscall(TGKILL, pid, gettid(), signum);

    if (ret_val == -1)
        return errno;

    return 0;
}

void thread_exit(void *retval){

    // when the main/parent thread calls thread_exit then use exit syscall to terminate main thread instead of return.
    if (getpid() == gettid())
        exit(0);

    thread_control_block *curr = get_info_tcb(threads_list, gettid());

    curr->result = retval;
    longjmp(curr->buf, 1);

}