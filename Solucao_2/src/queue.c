// Adaptado de
// https://www.geeksforgeeks.org/queue-set-1introduction-and-array-implementation/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/queue.h"

// Criacao da fila vazia com determinada capacidade
queue_t *createQueue(unsigned capacity)
{
    queue_t *queue = (queue_t *)malloc(
        sizeof(queue_t));
    queue->capacity = capacity;
    queue->front = queue->size = 0;

    queue->rear = capacity - 1;
    queue->array = (int *)malloc(
        queue->capacity * sizeof(int));

    pthread_mutex_init(&(queue->mutex), NULL);
    return queue;
}

// Retorna se a fila esta cheia caso o tamanho seja igual a capacidade
int isFull(queue_t *queue)
{
    int size;
    pthread_mutex_lock(&(queue->mutex));
    size = queue->size;
    pthread_mutex_unlock(&(queue->mutex));
    return size == queue->capacity;
}

// Fila esta vazia caso o tamanho seja zero
int isEmpty(queue_t *queue)
{
    int size;
    pthread_mutex_lock(&(queue->mutex));
    size = queue->size;
    pthread_mutex_unlock(&(queue->mutex));
    return size == 0;
}

// Adiciona um item no final da fila
void push(queue_t *queue, int item)
{
    pthread_mutex_lock(&(queue->mutex));
    if (queue->size == queue->capacity) {
        pthread_mutex_unlock(&(queue->mutex));
        return;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    pthread_mutex_unlock(&(queue->mutex));
}

// Remove um item da fila e retorna esse item
int pop(queue_t *queue)
{
    pthread_mutex_lock(&(queue->mutex));
    if (queue->size == 0) {
        pthread_mutex_unlock(&(queue->mutex));
        return INT_MIN;
    }
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    pthread_mutex_unlock(&(queue->mutex));
    return item;
}

// Retorna a parte da frente da fila
int front(queue_t *queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->front];
}

// Retorna a parte de tras da fila
int rear(queue_t *queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->rear];
}

// Destroi a fila
void destroy_queue(queue_t *queue) {
    pthread_mutex_destroy(&(queue->mutex));
    free(queue->array);
    free(queue);
}
