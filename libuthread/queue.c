#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "queue.h"

struct node {
    void* data;
    struct node* prev;
    struct node* next;
};


struct queue {
    struct node* head;
    struct node* tail;
    int size;
};

queue_t queue_create(void)
{
    queue_t queue=(queue_t)calloc(1,sizeof(struct queue));
    if(!queue){
        return NULL;
    }

    queue->head=NULL;
    queue->tail=NULL;
    queue->size=0;

    return queue;
}

int queue_destroy(queue_t queue)
{
    if(queue==NULL || queue->size>0){
        return -1;
    }

    free(queue);

    return 0;
}

static void* removeHead(queue_t queue){
    if(queue->size==0){
        return NULL;
    }

    struct node* oldHead = queue->head;

    void* data = oldHead->data;

    oldHead->data=NULL;

    if(queue->size==1){
        free(oldHead);

        queue->head = queue->tail = NULL;
        queue->size=0;

        return data;
    }

    struct node* newHead = oldHead->next;

    oldHead->next = NULL;

    free(oldHead);

    newHead->prev = NULL;

    queue->head=newHead;

    (queue->size)--;

    return data;
}

void* removeTail(queue_t queue){
     if(queue->size==0){
        return NULL;
     }

     struct node* oldTail = queue->tail;

     void* data = oldTail->data;

     oldTail->data = NULL;

     if(queue->size == 1){

         free(oldTail);

         queue->head = queue->tail = NULL;
         queue->size = 0;

         return data;
     }

     struct node* newTail = oldTail->prev;

     oldTail->prev=NULL;

     free(oldTail);

     newTail->next=NULL;

     queue->tail=newTail;

     (queue->size)--;

     return data;
}

int queue_enqueue(queue_t queue, void *data)
{
    if(queue==NULL || data==NULL){
        return -1;
    }

    struct node* newNode=(struct node*)calloc(1,sizeof(struct node));

    if(!newNode){
        return -1;
    }

    newNode->data=data;
    newNode->prev=NULL;
    newNode->next=NULL;

    if(queue->size==0){
        queue->head = queue->tail = newNode;
        (queue->size)++;
        return 0;
    }


    queue->tail->next=newNode;
    newNode->prev=queue->tail;
    queue->tail=newNode;

    (queue->size)++;

    return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
    if(queue==NULL || data==NULL || queue->size==0){
        return -1;
    }

    *data = removeHead(queue);

    return 0;
}


int queue_delete(queue_t queue, void *data)
{
    if(queue==NULL || data==NULL || queue->size == 0){
        return -1;
    }

    struct node* currentNode = queue->head;

    do{
        if(currentNode->data == data){

            if(currentNode==queue->head){
                removeHead(queue);
                return 0;
            }

            if(currentNode==queue->tail){
                removeTail(queue);
                return 0;
            }

            struct node* prevNode = currentNode->prev;
            struct node* nextNode = currentNode->next;

            currentNode->data=NULL;
            currentNode->prev=NULL;
            currentNode->next=NULL;

            free(currentNode);

            prevNode->next=nextNode;
            nextNode->prev=prevNode;

            (queue->size)--;

            return 0;
        }

        currentNode=currentNode->next;

    } while(currentNode!=NULL);

    return -1;

}

int queue_iterate(queue_t queue, queue_func_t func)
{
    if(queue==NULL || func==NULL){
        return -1;
    }

    struct node* currentNode=queue->head;

    while(currentNode!=NULL){
        struct node* next = currentNode->next;

        func(queue,currentNode->data);

        currentNode=next;
    }

    return 0;
}

int queue_length(queue_t queue)
{
    if(queue==NULL){
        return -1;
    }

    return queue->size;
}

