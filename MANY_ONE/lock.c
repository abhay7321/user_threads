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

