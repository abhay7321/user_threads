#include <stdio.h>
#include <unistd.h>
#include "lock.h"

static int test_and_set(int *value){
    int r, set = 1;
    asm("lock xchgl %0, %1"
        : "+m"(*value), "=a"(r)
        : "1"(set)
        : "cc");
    return r;
}

void spinlock_init(spinlock *sl){
    sl->flag = 0;
}

void thread_lock(spinlock *sl){
    while (test_and_set(&sl->flag))
        ;
}

void thread_unlock(spinlock *sl){
    sl->flag = 0;
}

void mutexlock_init(mutexlock *lock){
    lock->flag = 0;
}

void thread_mutex_lock(mutexlock *cm){
    thread_lock(&(cm->sl));
    while (cm->flag){
        thread_unlock(&(cm->sl));
        sleep(1);
        thread_lock(&(cm->sl));
    }
    cm->flag = 1;
    thread_unlock(&(cm->sl));
}

void thread_mutex_unlock(mutexlock *cm){
    thread_lock(&(cm->sl));
    cm->flag = 0;
    thread_unlock(&(cm->sl));
}
