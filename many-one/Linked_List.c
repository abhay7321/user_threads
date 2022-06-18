#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "Linked_List.h"


//Appends the given tcb that is passed as argument to thread_Linked_List at the end
int push_thread(thread_LL *LL, thread_control_block *tcb1){

    node_LL *curr = (node_LL *)malloc(sizeof(node_LL));
    
    if(!curr) return -1;
    
    curr->tcb = tcb1;
    curr->next = NULL;

    if(LL->head == NULL){
        LL->head = curr;
        LL->tail = curr;
        return 0;
    }
    LL->tail->next = curr;
    LL->tail = curr;
    return 0;
}

//Deleting node_LL of thread_id from the thread_LL
int delete_thread(thread_LL *LL, thread thread_id){
    node_LL * curr = LL->head;
    
    //If LL doesnt have any node return
    if(LL->head == NULL){
        return 0;
    }

    //If the given thread_id is present at head of LL
    if(LL->head->tcb->tid == thread_id){
        LL->head = curr->next;
        free(curr->tcb->stack_beginning);
        if(LL->head == NULL){
            LL->tail = NULL;
        }
        free(curr->tcb->context);
        free(curr->tcb->arr_waiting);
        free(curr->tcb);
        free(curr);
        return 0;
    }
    else{
        curr=curr->next;
        //Finding where the tcb of that thread_id is located
        while(curr->next){
            if(curr->next->tcb->tid == thread_id){
                free(curr->next->tcb->stack_beginning);

                //Node that is to be deleted
                node_LL* remove_node = curr->next;

                //If node to be removed is the last node of the LList
                if(remove_node == LL->tail){
                    LL->tail = curr;
                }
                //DELETING the remove_node
                curr->next = remove_node->next;
                
                free(remove_node->tcb->context);
                free((remove_node->tcb)->arr_waiting);
                free(remove_node->tcb);
                free(remove_node);
                return 0;
            }
            curr = curr->next;
        }
    }
    //If node_LL of thread_id is not present in the LL.
    return 0;
}


//Finds the first READY thread from the LL
//Deletes that thread from the LL
//Adds that thread at the end of LL
//Returns the first READY thread from LL
thread_control_block* get_first_ready_thread(thread_LL *LL){
    node_LL *curr = LL->head;

    //LL is empty then return
    if(curr == NULL){
        return NULL;
    }

    //IF LL has only one node
    if(curr->next == NULL){
        if(curr->tcb->state_of_thread == READY){
            return curr->tcb;
        }
        else{
            return NULL;
        }
    }

    //If the head node of LL is in READY STATE
    if(curr->tcb->state_of_thread == READY){
        thread_control_block* tcb_res =  curr->tcb;
        
        //Deleting the node from the LL
        LL->head = LL->head->next;

        //Shifting node at the end of LL
        push_thread(LL, tcb_res);

        free(curr);
        return tcb_res;
    }
    while(curr->next){
        node_LL* remove_node =  curr->next;
        if(remove_node->tcb->state_of_thread == READY){
            //Deleting the node from the LL
            curr->next = remove_node->next;

            thread_control_block* tcb_res = remove_node->tcb;

            //If removed node was the tail of the LL
            if(remove_node == LL->tail){
                LL->tail = curr;
            }

            //Shifting node at the end of LL
            push_thread(LL, tcb_res);

            free(remove_node);
            return tcb_res;
        }
        curr = curr->next;
    }
    return NULL;
}


//return the thread_control_block pointer that contais the given 'tid' from LL . If not present then returns NULL.
thread_control_block* get_tcb_of_tid(thread_LL *LL, thread thread_id){
    if(LL->head){
        node_LL *curr = LL->head;
        while(curr!=NULL){
            if(curr->tcb->tid == thread_id){
                return curr->tcb;
            }
            curr = curr->next;
        }
    }
    return NULL;
}


//Shows  thread_id and state of all the tcb that are stored in LL.
void show_all_thread_data(thread_LL *LL){
    node_LL *curr = LL->head;
    while(curr!=NULL){
        printf("tid = %ld \t\t\t state = %d \n", curr->tcb->tid, curr->tcb->state_of_thread);
        curr = curr->next;
    }
    printf("\n\n\n");
    return;
}


//Find the executing thread in Linkedlist
//make its state=READY 
//and shift it at the end of LinkedList
void change_state_shift(thread_LL *LL){
    node_LL *curr = LL->head;

    //If there are no nodes in LL return
    if(curr == NULL){
        return;
    }


    if(curr->tcb->state_of_thread == EXECUTING){
        thread_control_block* tcb_res =  curr->tcb;
        //Deleting the resulting node
        LL->head = LL->head->next;

        //Changing the state of resulting node
        tcb_res->state_of_thread = READY;

        //Shifting the node at the end
        push_thread(LL, tcb_res);
        free(curr);
        return;
    }
    while(curr->next){
        node_LL* remove_node =  curr->next;
        if(remove_node->tcb->state_of_thread == EXECUTING){
            //Deleting the resulting node
            curr->next = remove_node->next;

            thread_control_block* tcb_res = remove_node->tcb;

            //If removed node is the tail of LL
            if(remove_node == LL->tail){
                LL->tail = curr;
            }

            //Changing the state of resulting node
            tcb_res->state_of_thread = READY;

            //Shifting the node at the end
            push_thread(LL, tcb_res);

            free(remove_node);
            break;
        }
        curr = curr->next;
    }
    return;
}


//Find if there exits a node in the ThreadLinkedList that contains tcb
//If the node exits then shift it to the end of LL
void shift_tail(thread_LL *LL, thread_control_block* tcb){
    node_LL *curr = LL->head;

    //If there are no nodes in LL return
    if(curr == NULL){
        return;
    }

    //If first node of LL contains the given tcb
    if(curr->tcb->tid == tcb->tid){
        //Deleting the node
        LL->head = LL->head->next;

        //Pushing the deleted node at the end
        push_thread(LL, tcb);   

        free(curr);
        return;
    }

    //Search for the node in LL
    while(curr->next){
        struct node_LL* remove_node =  curr->next;
        if(remove_node->tcb->tid == tcb->tid){
            //Deleting the node
            curr->next = remove_node->next;

            //If the node to be removed is the tail node of LL
            if(remove_node == LL->tail){
                LL->tail = curr;
            }

            //Pushing the deleted node at the end
            push_thread(LL, tcb);

            free(remove_node);
            break;
        }
        curr = curr->next;
    }
    return;
}
