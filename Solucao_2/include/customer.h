#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

// Struct do cliente, possui um id e o semaforo associado a esse cliente
typedef struct customer_t {
    sem_t sem;
    int *id;
} customer_t;

customer_t *make_customer(int id);

int get_id(customer_t *customer);

void destroy_customer(customer_t *customer);

#endif