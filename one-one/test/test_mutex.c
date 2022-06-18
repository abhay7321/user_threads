#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "../thread.h"
#include "../lock.h"

long c = 0, c1 = 0, c2 = 0;

mutexlock sl;

void *thread1(void *arg)
{

    int run = 0;

    while (run < 50000)
    {
        thread_mutex_lock(&sl);
        c++;
        c1++;
        thread_mutex_unlock(&sl);
        run++;
    }
}

void *thread2(void *arg)
{

    int run = 0;

    while (run < 50000)
    {
        thread_mutex_lock(&sl);
        c++;
        c2++;
        thread_mutex_unlock(&sl);
        run++;
    }
}

int main()
{

    mutexlock_init(&sl);

    printf("Test 16: Testing mutex lock\n");
    thread t1, t2;
    thread_create(&t1, thread1, NULL, JOINABLE);
    thread_create(&t2, thread2, NULL, JOINABLE);

    thread_join(t1, NULL);
    thread_join(t2, NULL);

    if (c == c1 + c2)
        printf("TEST_CASE 16 PASSED\n\n");
    else
        printf("TEST_CASE 16 FAILED\n\n");

    return 0;
}