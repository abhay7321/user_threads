#include "thread.h"


typedef struct node_LL {
	struct node_LL *next;
	thread_control_block *tcb;
}node_LL;


typedef struct thread_LL {
	node_LL *head;
	node_LL *tail;
}thread_LL;


int push_thread(thread_LL *, thread_control_block *);
int delete_thread(thread_LL *, thread);
thread_control_block* get_tcb_of_tid(thread_LL *, thread);
void show_all_thread_data(thread_LL *);
void shift_tail(thread_LL *, thread_control_block *);
void change_state_shift(thread_LL *);
thread_control_block* get_first_ready_thread(thread_LL *);