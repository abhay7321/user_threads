#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "../thread.h"

void handler()
{
    printf("In the signal handler\n");
}

void *func1()
{
    // Do some work;
    int c = 1;
    for (int i = 0; i < 100000; i++)
    {
        c++;
    }
}

int main()
{
    thread td1, td2;

    //-----------------------------------------TEST 13-------------------------------------------------
    printf("Test 15: Send invalid signal\n");
    signal(SIGUSR1, handler);
    thread_create(&td1, func1, NULL, JOINABLE);
    if (thread_kill(td1, -1) == 0)
    {
        printf("TEST_CASE 13 FAILED\n\n");
    }
    else
    {
        printf("TEST_CASE 13 PASSED\n\n");
    }
    thread_join(td1, NULL);

    //-----------------------------------------TEST 14-------------------------------------------------
    printf("Test 16: Send Valid signal\n");
    thread_create(&td2, func1, NULL, JOINABLE);
    signal(SIGUSR1, handler);
    if (thread_kill(td2, SIGUSR1) == 0)
    {
        printf("TEST_CASE 14 PASSED\n\n");
    }
    else
    {
        printf("TEST_CASE 14 FAILED\n\n");
    }
    thread_join(td2, NULL);

    return 0;
}
