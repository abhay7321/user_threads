#include "thread.h"
#include <stdlib.h>

typedef struct node_LL
{
	struct node_LL *next;
	thread_control_block *tcb;
} node_LL;

typedef struct thread_LL
{
	node_LL *head;
	node_LL *tail;
} thread_LL;

int LL_push(thread_LL *, thread_control_block *);
void LL_deletenode(thread_LL *list, node_LL *node);
int LL_empty(thread_LL *);
thread_control_block *get_info_tcb(thread_LL *, int tid);
node_LL *get_node(thread_LL *list, int tid);