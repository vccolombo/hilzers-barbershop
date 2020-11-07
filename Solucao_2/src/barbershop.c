// Sistemas Operacionais - 2020.1
// Projeto de Implementação 1
// Profa. Dra. Kelen Cristiane Teixeira Vivaldini

// Eric Sales Vitor Junior. RA 758565. Engenharia de Computação.
// Fernanda Malheiros Assi. RA 743534. Engenharia de Computação.
// João Gabriel Viana Hirasawa. RA 759055. Engenharia de Computação.
// Pabolo Vinícius da Rosa Pires. RA 760648. Ciência da Computação.
// Víctor Cora Colombo. RA 727356. Engenharia de Computação.

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include "../include/queue.h"
#include "../include/customer.h"
#include "../include/barber.h"

#define TRUE 1
#define FALSE 0

// Parametros a serem utilizados na solucao
#define N_BARBERS 3
#define N_CUSTOMERS 25
#define SOFA_SIZE 4
#define BARBERSHOP_CAPACITY 20

sem_t sofa,                     // Lugares disponiveis no sofa
      barber;                   // Barbeiros disponiveis para cortar

pthread_mutex_t door,           // Protege a entrada e saida da loja
                cash_register;  // Protege a caixa registradora

// Contador de clientes na loja
int customers_count = 0;

// Fila para sentar no sofa
queue_t *sofa_queue;

// Indicador se o caixa esta ocupado
int cash_register_is_occupied = FALSE;

// Arrays de barbeiros e clientes
// A struct de barbeiro contem o id e sua fila do caixa
// A struct de cliente contem o id e o semaforo do cliente
barber_t *barbers[N_BARBERS];
customer_t *customers[N_CUSTOMERS];

// Thread do barbeiro
void *barber_routine(void *args);

// Thread do cliente
void *customer_routine(void *args);

// Inicializacao e destruicao de semaforos e mutexes
void initialize_mutexes();
void destroy_mutexes();
void initialize_semaphores();
void destroy_semaphores();

int main()
{
    int i;
    // Arrays de threads dos barbeiros e clientes
    pthread_t barbers_threads[N_BARBERS], customers_threads[N_CUSTOMERS];
    
    // Fila utilizada para o sofa
    sofa_queue = createQueue(SOFA_SIZE);

    // Inicializacao dos mutexes e semaforos
    initialize_mutexes();
    initialize_semaphores();

    // Criacao dos barbeiros e suas threads
    for (i = 0; i < N_BARBERS; i++)
    {
        int id = i;
        barbers[i] = make_barber(id);
        if (pthread_create(&barbers_threads[i], NULL, barber_routine, (void *) barbers[i]->id) != 0) {
            perror("Problema na criacao da thread\n");
            exit(EXIT_FAILURE);
        }
    }

    // Criacao dos clientes e suas threads
    for (i = 0; i < N_CUSTOMERS; i++)
    {
        int id = i;
        customers[i] = make_customer(id);
        if (pthread_create(&customers_threads[i], NULL, customer_routine, (void *) customers[i]->id) != 0) {
            perror("Problema na criacao da thread\n");
            exit(EXIT_FAILURE);
        }
    }

    // Join nos clientes
    for (i = 0; i < N_CUSTOMERS; i++)
    {
        if (pthread_join(customers_threads[i], NULL) != 0)
        {
            perror("Problema no join\n");
            exit(EXIT_FAILURE);
        }
    }

    // Clientes ja finalizados
    sleep(1);
    printf("Todos os clientes foram atendidos\n");
    
    // Finalizando os barbeiros
    for (i = 0; i < N_BARBERS; i++)
    {
        printf("Barbeiro %d encerrou o seu expediente\n", i);
        pthread_cancel(barbers_threads[i]);
    }

    // Liberando recursos
    for (i = 0; i < N_BARBERS; i++) {
        destroy_barber(barbers[i]);
    }
    for (i = 0; i < N_CUSTOMERS; i++) {
        destroy_customer(customers[i]);
    }
    destroy_queue(sofa_queue);
    destroy_mutexes();
    destroy_semaphores();

    return 0;
}

// Barbeiro - Thread principal
void *barber_routine(void *args)
{
    int id = *((int *) args);

    // Variaveis de clientes que serao utilizadas na rotina
    unsigned int customer_id;
    customer_t *customer;

    printf("Barbeiro %d chegou para trabalhar\n", id);
    while (1)
    {
        // Mutex do caixa e trancado
        pthread_mutex_lock(&cash_register);

        // Se houver um cliente na fila de pagamento do barbeiro e o caixa estiver vazio
        if (!isEmpty(barbers[id]->cash_register_queue) && cash_register_is_occupied == FALSE) {
            printf("Barbeiro %d foi ao caixa\n", id);

            // Coloca o caixa em ocupado
            cash_register_is_occupied = TRUE;
            pthread_mutex_unlock(&cash_register);

            // Esvazia a fila de pagamentos
            while(!isEmpty(barbers[id]->cash_register_queue)) {
                // Retira um cliente dessa fila e cobra dele
                customer_id = pop(barbers[id]->cash_register_queue);
                customer = customers[customer_id];
                printf("Barbeiro %d cobrou o cliente %d\n", id, customer_id);
                sem_post(&(customer->sem));
            }

            // Coloca o caixa em livre
            pthread_mutex_lock(&cash_register);
            cash_register_is_occupied = FALSE;
            pthread_mutex_unlock(&cash_register);
        } else if (!isEmpty(sofa_queue)) {      // Caso nao haja clientes na fila de pagamento mas haja para cortar
            printf("Barbeiro %d foi cortar cabelo\n", id);

            // Avisa que esta disponivel para cortar
            sem_post(&barber);

            // Retira um cliente da fila
            customer_id = pop(sofa_queue);

            // Desbloqueia o mutex do caisa
            pthread_mutex_unlock(&cash_register);

            // Pega-se o semaforo do cliente e avisa que esta pronto para cortar
            customer = customers[customer_id];
            sem_post(&(customer->sem));

            sleep(1);

            // Avisa que o corte finalizou e o manda para a fila de pagamento
            sem_post(&(customer->sem));
            push(barbers[id]->cash_register_queue, get_id(customer));
        } else {
            // Caixa ocupado e sofa vazio, ou esperando os outros barbeiros terminarem
            // para apenas encerrar
            pthread_mutex_unlock(&cash_register);
            sleep(1);
        }
    }
}

// Cliente - Thread principal
void *customer_routine(void *arg)
{
    int id = *((int *) arg);
    customer_t *customer = customers[id];

    // Cliente chegou na porta, bloqueia o mutex
    pthread_mutex_lock(&door);
    // Caso o local esteja cheio, desbloqueia o mutex e vai embora
    if (customers_count == BARBERSHOP_CAPACITY) {
        pthread_mutex_unlock(&door);
        printf("Cliente %d saiu sem cortar o cabelo\n", id);
        pthread_exit((void *) EXIT_SUCCESS);
    }
    // Incremento do contador de clientes
    customers_count++;
    pthread_mutex_unlock(&door);
    printf("Cliente %d entrou na loja\n", id);

    // Cliente aguarda liberar lugar no sofa
    sem_wait(&sofa);
    // Quando disponivel, se coloca na fila do sofa
    push(sofa_queue, id);
    printf("Cliente %d sentou no sofa\n", id);

    // Cliente aguarda um barbeiro estar disponivel
    sem_wait(&barber);
    // Cliente aguarda que o barbeiro o chame
    sem_wait(&(customer->sem));
    // Quando e chamado, libera a vaga no sofa
    sem_post(&sofa);
    printf("Cliente %d esta cortando o cabelo\n", id);

    // Cliente aguarda que o barbeiro sinalize que terminou
    sem_wait(&(customer->sem));
    printf("Cliente %d teve o cabelo cortado\n", id);

    // Cliente aguarda que o barbeiro permita pagar
    printf("Cliente %d esta esperando para pagar\n", id);
    sem_wait(&(customer->sem));

    // Cliente sai da loja, diminui o contador
    pthread_mutex_lock(&door);
    customers_count--;
    pthread_mutex_unlock(&door);
    printf("Cliente %d saiu apos cortar o cabelo\n", id);

    pthread_exit((void *) EXIT_SUCCESS);
}

// Inicializacao dos mutexes
void initialize_mutexes() {
    pthread_mutex_init(&door, NULL);
    pthread_mutex_init(&cash_register, NULL);
}

// Destruicao dos mutexes
void destroy_mutexes() {
    pthread_mutex_destroy(&door);
    pthread_mutex_destroy(&cash_register);
}

// Inicializacao dos semaforos gerais
void initialize_semaphores() {
    sem_init(&sofa, 0, SOFA_SIZE);
    sem_init(&barber, 0, N_BARBERS);
}

// Destruicao dos semaforos gerais
void destroy_semaphores() {
    sem_destroy(&sofa);
    sem_destroy(&barber);
}