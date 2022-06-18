#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "../thread.h"

void *func1()
{
    int *r = (int *)malloc(sizeof(int));

    *r = 1;
    return r;
}

void *func2()
{
    int *r2 = (int *)malloc(sizeof(int));

    *r2 = 1;
    thread_exit(r2);
}

int main()
{

    thread td1, td2;

    //-----------------------------------------TEST 11-------------------------------------------------
    printf("Test 13: Returning result from routine function\n");
    thread_create(&td1, func1, NULL, JOINABLE);
    int *result;
    if (thread_join(td1, (void *)&result) == 0)
    {
        if (*result == 1)
            printf("TEST_CASE 11 PASSED\n\n");
        else
            printf("TEST_CASE 11 FAILED\n\n");
    }
    else
    {
        printf("TEST_CASE 11 FAILED\n\n");
    }

    //-----------------------------------------TEST 12-------------------------------------------------
    printf("Test 14: Using thread_exit to return result from routine function\n");
    thread_create(&td2, func2, NULL, JOINABLE);
    if (thread_join(td2, (void *)&result) == 0)
    {
        if (*result == 1)
            printf("TEST_CASE 12 PASSED\n\n");
        else
            printf("TEST_CASE 12 FAILED\n\n");
    }
    else
    {
        printf("TEST_CASE 12 FAILED\n\n");
    }

    return 0;
}
