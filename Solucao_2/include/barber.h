#ifndef BARBER_H
#define BARBER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "queue.h"

// Struct do barbeiro, possui um id e a fila de pagamentos desse barbeiro
typedef struct barber_t {
    int *id;
    queue_t *cash_register_queue;
} barber_t;

barber_t *make_barber(int id);

void destroy_barber(barber_t *barber);

#endif