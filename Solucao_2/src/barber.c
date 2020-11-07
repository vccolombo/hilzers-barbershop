#include "../include/barber.h"

// Cria um novo barbeiro da struct, com um id e uma fila de pagamento
barber_t *make_barber(int id) {
    barber_t *new_barber = malloc(sizeof(barber_t));
    new_barber->id = malloc(sizeof(int));
    *new_barber->id = id;
    new_barber->cash_register_queue = createQueue(10);
    
    return new_barber;
}

// Destroi o barbeiro
void destroy_barber(barber_t *barber) {
    destroy_queue(barber->cash_register_queue);
    free(barber->id);
    free(barber);
}