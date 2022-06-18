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

void *func2()
{
    sleep(2);
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
    thread_exit(total);
}

void *TOTAL_MARKS2(void *args)
{

    marks *m1 = (marks *)args;
    int *total = (int *)malloc(sizeof(int));
    *total = m1->DSA + m1->DBMS + m1->OS + m1->CN;
    return total;
}

int main()
{

    thread td1, td2, td3, td4;

    //-----------------------------------------TEST 6-------------------------------------------------
    printf("Test 6: Join with valid JOINABLE thread\n");
    thread_create(&td1, func1, NULL, JOINABLE);
    if (thread_join(td1, NULL) == 0)
    {
        printf("TEST_CASE 6 PASSED\n\n");
    }
    else
    {
        printf("TEST_CASE 6 FAILED\n\n");
    }

    //-----------------------------------------TEST 7-------------------------------------------------
    printf("Test 7: Join with invalid thread id\n");
    thread_create(&td2, func1, NULL, JOINABLE);
    if (thread_join(-1, NULL) == 0)
    {
        printf("TEST_CASE 7 FAILED\n\n");
    }
    else
    {
        printf("TEST_CASE 7 PASSED\n\n");
    }

    //-----------------------------------------TEST 8-------------------------------------------------
    printf("Test 8: Join with already joined thread\n");

    if (thread_join(td1, NULL) == 0)
    {
        printf("TEST_CASE 8 FAILED\n\n");
    }
    else
    {
        printf("TEST_CASE 8 PASSED\n\n");
    }

    //-----------------------------------------TEST 9-------------------------------------------------
    printf("Test 9: Join with detached thread\n");

    thread td5;
    thread_create(&td5, func2, NULL, DETACHED);

    if (thread_join(td5, NULL) == 0)
    {
        printf("TEST_CASE 9 FAILED\n\n");
    }
    else
    {
        printf("TEST_CASE 9 PASSED\n\n");
    }

    //-----------------------------------------TEST 10-A-------------------------------------------------
    printf("Test 10-A: Joining on a thread and collecting exit status by returning result from routine function\n");

    thread td6;
    int *result;

    marks M;
    M.DSA = 5;
    M.DBMS = 10;
    M.CN = 15;
    M.OS = 20;

    thread_create(&td6, TOTAL_MARKS, &M, JOINABLE);
    if (thread_join(td6, (void *)&result) == 0)
    {
        if (*result == 50)
            printf("TEST_CASE 10-A PASSED\n\n");
        else
            printf("TEST_CASE 10-A FAILED\n\n");
    }
    else
    {
        printf("TEST_CASE 10-A FAILED\n\n");
    }

    //-----------------------------------------TEST 10-B-------------------------------------------------
    printf("Test 10-B: Joining on a thread and collecting exit status by using thread_exit in routine function\n");

    thread td7;
    int *result2;

    marks M2;
    M2.DSA = 5;
    M2.DBMS = 10;
    M2.CN = 15;
    M2.OS = 20;

    thread_create(&td7, TOTAL_MARKS2, &M2, JOINABLE);
    if (thread_join(td7, (void *)&result) == 0)
    {
        if (*result == 50)
            printf("TEST_CASE 10-B PASSED\n\n");
        else
            printf("TEST_CASE 10-B FAILED\n\n");
    }
    else
    {
        printf("TEST_CASE 10-B FAILED\n\n");
    }

    return 0;
}
