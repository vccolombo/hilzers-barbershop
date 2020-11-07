#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "../include/customer.h"

// Cria um novo cliente da struct, com um id e um semaforo associado
customer_t *make_customer(int id) {
    customer_t *new_customer = malloc(sizeof(customer_t));
    new_customer->id = malloc(sizeof(int));
    *new_customer->id = id;
    sem_init(&(new_customer->sem), 0, 0);

    return new_customer;
}

// Retorna o id do cliente
int get_id(customer_t *customer) {
    return *customer->id;
}

// Destroi o cliente
void destroy_customer(customer_t *customer) {
    sem_destroy(&(customer->sem));
    free(customer->id);
    free(customer);
}
