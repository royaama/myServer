#include "Queue.h"
#define SUC 1
#define FAIL 0

Queue* initQueue()
{
    Queue* new_queue = malloc(sizeof(Queue));
    new_queue->size = 0;
    new_queue->head = NULL;
    new_queue->tail = NULL;
    return new_queue;
}

node* getHead(Queue* my_queue)
{
    return my_queue->head;
}

Request* getFirstRequest(Queue* my_queue)
{
    return my_queue->head->data;
}

int dropHead(Queue* my_queue)
{
    if(my_queue == NULL)
        return SUC;
    if(my_queue->size != 0){
        removeFromQueue(my_queue,my_queue->head);
    }
    return SUC;
}

node* insertBack(Queue* my_queue , Request* data)
{ //last element is always null so add it before the null
    node* tempTail = my_queue->tail;
    node* newNode = malloc(sizeof (newNode));
    newNode->data = data;
    newNode->previous = NULL;
    my_queue->tail = newNode;
    if(my_queue->size == 0)
    {
        my_queue->head = newNode;
        newNode->next = NULL;
        my_queue->size++;
        return newNode;
    }
    newNode->next = tempTail;
    tempTail->previous = newNode;
    my_queue->size++;
    return newNode;
}

int removeFromQueue(Queue* my_queue, node* nodeToDelete)
{
    if(my_queue == NULL || nodeToDelete == NULL)
        return SUC;

    int is_first = 0;
    int is_last = 0;
    if(nodeToDelete->previous == NULL)
        is_last = 1;
    if(nodeToDelete->next == NULL)
        is_first = 1;

    if(!is_first && !is_last) // not last and not first
    {
        nodeToDelete->next->previous=nodeToDelete->previous;
        nodeToDelete->previous->next=nodeToDelete->next;
    }
    else if(!is_last) // first and not last
    {
        nodeToDelete->previous->next=NULL;
        my_queue->head = nodeToDelete->previous;

    }
    else if(!is_first) // last and not first
    {
        nodeToDelete->next->previous=NULL;
        my_queue->tail = nodeToDelete->next;
    }
    else // first and last
    {
        my_queue->tail=NULL;
        my_queue->head=NULL;
    }
    free(nodeToDelete);
    my_queue->size--;
    return SUC;
}

int removeByRandomIndex(Queue* my_queue, int index_to_remove)
{
    if(my_queue == NULL)
        return SUC;
    node* node_to_remove = my_queue->tail;
    int current_index = 0;

    while(current_index <= index_to_remove && node_to_remove != NULL)
    {
        if(current_index == index_to_remove)
        {
            Close(node_to_remove->data->confd);
            free(node_to_remove->data);
            removeFromQueue(my_queue, node_to_remove);
            return SUC;
        }
        node_to_remove = node_to_remove->next;
        current_index++;
    }
   return SUC;
}

Request* getByRandomIndex(Queue* my_queue, int i){
    if(my_queue == NULL)
        return NULL;
    node* node_to_return = my_queue->tail;
    int current_index = 0;
    while(current_index <= i && node_to_return != NULL)
    {
        if(current_index == i)
        {
            return node_to_return->data;
        }
        node_to_return = node_to_return->next;
        current_index++;
    }
    return NULL;
}

int delete_aux_waiting(Queue* my_queue){
    if(my_queue == NULL){
        return SUC;
    }
    dropHead(my_queue);
    delete_aux_waiting(my_queue);
    return SUC;
}

int delete_waiting_queue(Queue* my_queue)
{
    if(delete_aux_waiting(my_queue)){
        free(my_queue);
        return SUC;
    }
    return SUC;
}


int delete_aux_handling(Queue* my_queue){
    if(my_queue == NULL){
        return SUC;
    }
    Close(my_queue->head->data->confd);
    free(my_queue->head->data);
    dropHead(my_queue);
    delete_aux_handling(my_queue);
    return SUC;
}

int delete_handling_queue(Queue* my_queue)
{
    if(delete_aux_handling(my_queue)){
        free(my_queue);
        return SUC;
    }
    return SUC;
}