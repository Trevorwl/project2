#include <stdint.h>
#include <stdlib.h>

#include "queue.h"

typedef struct node {
    void *data;
    struct node *next;
} node_t;

struct queue {
    node_t *head;
    node_t *tail;
    int length;
};

queue_t queue_create(void)
{
    queue_t q = calloc(1,sizeof(struct queue));
    if (!q)
        return NULL;

    q->head = q->tail = NULL;
    q->length = 0;
    return q;
}

int queue_destroy(queue_t queue)
{
    if (!queue || queue->length > 0)
        return -1;

    free(queue);
    return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
    if (!queue || !data)
        return -1;

    node_t *new_node = calloc(1,sizeof(node_t));
    if (!new_node)
        return -1;

    new_node->data = data;
    new_node->next = NULL;

    if (queue->length == 0){
        queue->head = queue->tail = new_node;
    }else {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }

    (queue->length)++;
    return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
    if (!queue || !data || queue->length == 0)
        return -1;

    node_t *old_head = queue->head;
    *data = old_head->data;

    queue->head = old_head->next;
    if (!queue->head)
        queue->tail = NULL;

    free(old_head);
    (queue->length)--;
    return 0;
}

int queue_delete(queue_t queue, void *data)
{
    if (!queue || !data || queue->length == 0)
        return -1;

    node_t *curr = queue->head;
    node_t *prev = NULL;

    while (curr) {
        if (curr->data == data) {
            if (prev)
                prev->next = curr->next;
            else
                queue->head = curr->next;

            if (curr == queue->tail)
                queue->tail = prev;

            free(curr);
            (queue->length)--;
            return 0;
        }

        prev = curr;
        curr = curr->next;
    }

    return -1;
}

int queue_iterate(queue_t queue, queue_func_t func)
{
    if (!queue || !func)
        return -1;

    node_t *curr = queue->head;
    while (curr) {
        node_t *next = curr->next;  // store next in case `curr` gets deleted
        func(queue, curr->data);
        curr = next;
    }

    return 0;
}


int queue_length(queue_t queue)
{
    if (!queue)
        return -1;

    return queue->length;
}

