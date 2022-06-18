#include "Linked_List.h"
#include "lock.h"
#include <stdio.h>

int LL_push(thread_LL *list, thread_control_block *tcb)
{

	thread_mutex_lock(&LL_lock);

	node_LL *new_node = (node_LL *)malloc(sizeof(node_LL));

	if (!new_node)
	{
		thread_mutex_unlock(&LL_lock);
		return -1;
	}

	new_node->tcb = tcb;
	new_node->next = NULL;

	if (list->head == NULL)
	{
		list->head = new_node;
		list->tail = new_node;
	}
	else
	{
		(list->tail)->next = new_node;
		list->tail = new_node;
	}
	list->tail->next = NULL;

	thread_mutex_unlock(&LL_lock);

	return 0;
}

void LL_deletenode(thread_LL *list, node_LL *node)
{

	thread_mutex_lock(&LL_lock);

	node_LL *prev, *curr;
	curr = list->head;
	prev = NULL;

	while (curr)
	{

		if ((curr->tcb)->tid == (node->tcb)->tid)
		{

			if (list->head == list->tail)
			{

				list->head = NULL;
				list->tail = NULL;
				break;
			}

			// if the node is first node in the LL
			if (prev == NULL)
			{

				list->head = curr->next;
			}
			// if the node is last node
			else if (curr->next == NULL)
			{

				prev->next = NULL;
				list->tail = prev;
			}
			// if the node is in between
			else
			{

				prev->next = curr->next;
			}

			break;
		}

		prev = curr;
		curr = curr->next;
	}

	thread_mutex_unlock(&LL_lock);

	return;
}

int LL_empty(thread_LL *list)
{

	thread_mutex_lock(&LL_lock);
	int r;
	if (list->head == NULL)
		r = 1;
	else
		r = 0;

	thread_mutex_unlock(&LL_lock);

	return r;
}

thread_control_block *get_info_tcb(thread_LL *list, int tid)
{

	thread_mutex_lock(&LL_lock);
	node_LL *curr;
	curr = (list->head);
	while (curr != NULL)
	{
		if ((curr->tcb)->tid == tid)
		{
			thread_mutex_unlock(&LL_lock);
			return curr->tcb;
		}
		curr = curr->next;
	}
	thread_mutex_unlock(&LL_lock);
	return NULL;
}

node_LL *get_node(thread_LL *list, int tid)
{

	thread_mutex_lock(&LL_lock);

	node_LL *temp = list->head;

	while (temp)
	{

		if ((temp->tcb)->tid == tid)
		{

			thread_mutex_unlock(&LL_lock);
			return temp;
		}
		temp = temp->next;
	}
	thread_mutex_unlock(&LL_lock);

	return NULL;
}
