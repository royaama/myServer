#ifndef Queue_H
#define Queue_H

#include "segel.h"

struct Request
{
    int confd;
    struct timeval arrive;
    struct timeval dispatch;
};
typedef struct Request Request;

struct node {

    Request* data;
    struct node* next;
    struct node* previous;
};
typedef struct node node;

struct queue
{
    int size;
    struct node *head;
    struct node *tail;
};

typedef struct queue Queue;

Queue* initQueue();
node* insertBack(Queue* my_queue, Request* data);
node* getHead(Queue* my_queue);
Request* getByRandomIndex(Queue* my_queue, int i);
int dropHead(Queue* my_queue);
int removeFromQueue(Queue* my_queue, node* nodeToDelete);
int removeByRandomIndex(Queue* my_queue, int index);
int delete_handling_queue(Queue* my_queue);
int delete_waiting_queue(Queue* my_queue);


#endif //Queue_H