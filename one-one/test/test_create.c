#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "../thread.h"

void *func1()
{
    int r = 1;
    for (int i = 0; i < 5; i++)
    {
        r++;
    }
}

typedef struct marks
{
    int DSA;
    int DBMS;
    int CN;
    int OS;
} marks;

void *TOTAL_MARKS(void *args)
{

    marks *m1 = (marks *)args;
    int *total = (int *)malloc(sizeof(int));
    *total = m1->DSA + m1->DBMS + m1->OS + m1->CN;
    return total;
}

int main()
{

    thread td1;
    //-----------------------------------------TEST 1-------------------------------------------------
    printf("Test 1: Creating a thread with all valid thread arguments\n");

    printf("thread_main() created thread()\n");

    if (thread_create(&td1, func1, NULL, JOINABLE) == 0)
    {
        printf("TEST_CASE 1 PASSED\n\n");
        thread_join(td1, NULL);
    }
    else
    {
        printf("TEST_CASE 1 FAILED\n\n");
    }

    //-----------------------------------------TEST 2-------------------------------------------------

    printf("Test 2: Creating a thread with invalid thread argument\n");

    if (thread_create(NULL, func1, NULL, JOINABLE) == 0)
    {
        printf("TEST_CASE 2 FAILED\n\n");
    }
    else
    {
        printf("TEST_CASE 2 PASSED\n\n");
    }

    //-----------------------------------------TEST 3-------------------------------------------------
    printf("Test 3: Creating a thread with invalid start function\n");
    thread td3;

    if (thread_create(&td3, NULL, NULL, JOINABLE) == 0)
    {
        printf("TEST_CASE 3 FAILED\n\n");
    }
    else
    {
        printf("TEST_CASE 3 PASSED\n\n");
    }

    //-----------------------------------------TEST 4-------------------------------------------------
    printf("Test 4: Creating a thread with routine function arguments \n");

    thread td4;
    int *result;

    marks M;
    M.DSA = 5;
    M.DBMS = 10;
    M.CN = 15;
    M.OS = 20;

    if (thread_create(&td4, TOTAL_MARKS, &M, JOINABLE) == 0)
    {
        printf("TEST_CASE 4 PASSED\n\n");
        thread_join(td4, (void *)&result);
    }
    else
    {
        printf("TEST_CASE 4 FAILED\n\n");
    }
}
