#ifndef QUEUE_H
#define QUEUE_H

// Adaptado de
// https://www.geeksforgeeks.org/queue-set-1introduction-and-array-implementation/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Struct da fila
typedef struct queue_t
{
    int front, rear, size;
    unsigned capacity;
    int *array;
    pthread_mutex_t mutex;
} queue_t;

queue_t *createQueue(unsigned capacity);

int isFull(queue_t *queue);
int isEmpty(queue_t *queue);

void push(queue_t *queue, int item);
int pop(queue_t *queue);

int front(queue_t *queue);
int rear(queue_t *queue);

void destroy_queue(queue_t *queue);

#endif