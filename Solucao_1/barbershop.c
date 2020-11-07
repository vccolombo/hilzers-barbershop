// Sistemas Operacionais - 2020.1
// Projeto de Implementação 1
// Profa. Dra. Kelen Cristiane Teixeira Vivaldini

// Eric Sales Vitor Junior. RA 758565. Engenharia de Computação.
// Fernanda Malheiros Assi. RA 743534. Engenharia de Computação.
// João Gabriel Viana Hirasawa. RA 759055. Engenharia de Computação.
// Pabolo Vinícius da Rosa Pires. RA 760648. Ciência da Computação.
// Víctor Cora Colombo. RA 727356. Engenharia de Computação.

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "queue.c"

// Macros para as chamadas de sleep
#define HAIRCUT_TIME 5
#define MAX_TIME_BEFORE_ENTERING_SHOP 10
#define SEAT_TIME 1
#define PAY_TIME 1
#define BARBER_DRINKING_WATER 1

sem_t capacity_sem,                    // Capacidade maxima da loja
      sofa_sem,                        // Lugares no sofa
      clientWaitingForSofa_sem,        // Clientes esperando sofa
      clientWaitingForBarberChair_sem; // Clientes esperando cadeirinha

pthread_mutex_t standupToSofa_mutex,   // Protege a fila para sentar no sofa
                sofaToBarber_mutex,    // Protege a fila para cortar o cabelo
                payment_mutex;         // Protege o caixa

struct Queue *standupToSofaQueue,      // Fila para sentar no sofa
             *sofaToBarberQueue;       // Fila para cortar o cabelo

// Struct de parametros para receber do terminal, usada em
// setupParameters e na criacao de semaforos
typedef struct Parameters {
    unsigned long N_CLIENTS, N_BARBERS, N_SOFA, MAX_CAPACITY;
} Parameters;

Parameters setupParameters(char const* argv[]);

// Inicializacao e destruicao de semaforos, mutexes e filas
void initSemaphoresAndMutexes(Parameters params);
void destroyMutexes();
void initQueues();

// Barbeiro - Thread e criacao
void* barberThread(void* args);
pthread_t createBarberThread(unsigned long id);

// Barbeiro - Funcionamento
Client* dequeueClientFromSofa();
void callClientToChair(unsigned long id, Client* client);
void cutHair(unsigned long id, Client* client);
void acceptPayment(unsigned long id, Client* client);

// Secretaria - Thread e criacao
void* secretariaThread(void* args);
pthread_t createSecretariaThread(unsigned long id);

// Secretaria - Funcionamento
void sendClientToSofa();

// Cliente - Thread, criacao e destruicao
void* clientThread(void* args);
pthread_t createClientThread(unsigned long id);
Client* initClientSemaphores();
void freeClientSemaphores(Client* client);

// Cliente - Funcionamento
void enterShop(unsigned long id, Client* client);
void waitAndSeatOnSofa(unsigned long id, Client* client);
void seatOnBarberChair(unsigned long id, Client* client);
void getHaircut(unsigned long id, Client* client);
void pay(unsigned long id, Client* client);
void leaveShop(unsigned long id, short int success);

// Rotina de criacao de threads
void doThreadsCreationAndJoins(Parameters params);

int main(int argc, char const* argv[]) {
    if (argc != 5) {
        printf("Uso: ./main N_CLIENTS N_BARBERS N_SOFA_SEATS MAX_CAPACITY\n");
        exit(1); 
    }

    // Inicializacao dos parametros a partir dos argumentos
    // recebidos no terminal
    Parameters params = setupParameters(argv);

    // Inicializacao dos semaforos com os parametros
    initSemaphoresAndMutexes(params);

    // Inicializacao das filas
    initQueues();

    // Criacao e join das threads
    doThreadsCreationAndJoins(params);

    // Destruicao dos mutexes para termino do programa
    destroyMutexes();

    return 0;
}

// Recebe os parametros do terminal e cria uma struct
// de Parameters com eles
Parameters setupParameters(char const* argv[]) {
    Parameters params;
    params.N_CLIENTS = atoi(argv[1]);
    params.N_BARBERS = atoi(argv[2]);
    params.N_SOFA = atoi(argv[3]);
    params.MAX_CAPACITY = atoi(argv[4]);

    return params;
}

// Inicializa os semaforos e mutexes globais a partir
// dos parametros fornecidos
void initSemaphoresAndMutexes(Parameters params) {
    sem_init(&capacity_sem, 0, params.MAX_CAPACITY);
    sem_init(&sofa_sem, 0, params.N_SOFA);
    sem_init(&clientWaitingForSofa_sem, 0, 0);
    sem_init(&clientWaitingForBarberChair_sem, 0, 0);

    pthread_mutex_init(&standupToSofa_mutex, NULL);
    pthread_mutex_init(&sofaToBarber_mutex, NULL);
    pthread_mutex_init(&payment_mutex, NULL);
}

// Destroi os mutexes
void destroyMutexes() {
    pthread_mutex_destroy(&standupToSofa_mutex);
    pthread_mutex_destroy(&sofaToBarber_mutex);
    pthread_mutex_destroy(&payment_mutex);
}

// Inicializa as filas globais utilizadas
void initQueues() {
    standupToSofaQueue = createQueue();
    sofaToBarberQueue = createQueue();
}

// Barbeiro - Thread e criacao

// Thread principal do barbeiro. Recebe como argumento
// um id.
void* barberThread(void* args) {
    unsigned long id = (unsigned long) args;
    printf("Barbeiro %ld chegou para trabalhar\n", id);

    while (1) {
        // Espera pelo sinal de um cliente esperando no sofa
        sem_wait(&clientWaitingForBarberChair_sem);

        // Remove o cliente do sofa e entao realiza as rotinas
        Client* client = dequeueClientFromSofa();
        callClientToChair(id, client);
        cutHair(id, client);
        acceptPayment(id, client);
        
        // Descansa por um tempo
        sleep(BARBER_DRINKING_WATER);
    }

    return NULL;
}

// Cria e retorna a thread de barbeiro com o id fornecido
pthread_t createBarberThread(unsigned long id) {
    pthread_t thr;
    pthread_create(&thr, NULL, barberThread, (void*)id);

    return thr;
}

// Barbeiro - Funcionamento

// Retira um cliente do sofa e retorna esse cliente
Client* dequeueClientFromSofa() {
    // Bloqueia o mutex para mexer na fila
    pthread_mutex_lock(&sofaToBarber_mutex);
    // Guarda e remove o cliente da fila
    Client* client = sofaToBarberQueue->front->key;
    deQueue(sofaToBarberQueue);
    pthread_mutex_unlock(&sofaToBarber_mutex);

    // Libera o lugar no sofa
    sem_post(&sofa_sem);

    return client;
}

// Avisa o cliente de que pode sentar na cadeira
void callClientToChair(unsigned long id, Client* client) {
    printf("Barbeiro %ld chamou um cliente para a cadeira\n", id);
    sem_post(client->shopToClient_sem);
}

// Espera pelo cliente e corta o cabelo dele
void cutHair(unsigned long id, Client* client) {
    // Aguarda o cliente sinalizar que esta pronto para cortar
    sem_wait(client->clientToShop_sem);
    printf("Barbeiro %ld esta cortando cabelo\n", id);
    sleep(HAIRCUT_TIME);
    // Avisa o cliente que terminou de cortar
    sem_post(client->shopToClient_sem);
}

// Espera o cliente pagar e recebe o pagamento
void acceptPayment(unsigned long id, Client* client) {
    sem_wait(client->clientToShop_sem);
    // Bloqueia o mutex para exclusividade mutua no caixa
    pthread_mutex_lock(&payment_mutex);
    sleep(PAY_TIME);
    printf("Barbeiro %ld recebeu pagamento\n", id);
    pthread_mutex_unlock(&payment_mutex);

    // Avisa o cliente que pode ir embora
    sem_post(client->shopToClient_sem);
}

// Secretaria - Thread e criacao

// Thread principal da secretaria. Recebe como argumento um id
void* secretariaThread(void* args) {
    unsigned long id = (unsigned long) args;
    printf("Secretaria %ld chegou para trabalhar\n", id);

    while (1) {
        // Aguarda que um cliente entre na loja e espere pelo sofa
        sem_wait(&clientWaitingForSofa_sem);

        // Aguarda que haja um local livre no sofa
        sem_wait(&sofa_sem);

        sendClientToSofa();
    }

    return NULL;
}

// Cria e retorna uma thread da secretaria com o id fornecido
pthread_t createSecretariaThread(unsigned long id) {
    pthread_t thr;
    pthread_create(&thr, NULL, secretariaThread, (void*)id);

    return thr;
}

// Secretaria - Funcionamento

// Envia um cliente que esta em pe para o sofa
void sendClientToSofa() {
    // Usa o mutex para mexer na fila dos clientes em pe
    pthread_mutex_lock(&standupToSofa_mutex);
    // Guarda e remove um cliente da fila em pe
    Client* movingClient = standupToSofaQueue->front->key;
    deQueue(standupToSofaQueue);
    pthread_mutex_unlock(&standupToSofa_mutex);

    // Avisa o cliente retirado de que pode sentar no sofa
    sem_post(movingClient->shopToClient_sem);
}

// Cliente - Thread, criacao e destruicao

// Thread principal do cliente. Recebe como argumento um id
void* clientThread(void* args) {
    unsigned long id = (unsigned long)args;

    // Inicializa os semaforos do cliente
    Client* client = initClientSemaphores();

    // Espera um tempo aleatorio antes de entrar na loja
    sleep(random() % MAX_TIME_BEFORE_ENTERING_SHOP);
    printf("Cliente %ld entrou na loja\n", id);

    // Tenta entrar na loja, caso nao haja espaco apenas sai
    if (sem_trywait(&capacity_sem) == 0) {
        // Rotina padrao do cliente
        enterShop(id, client);
        waitAndSeatOnSofa(id, client);
        seatOnBarberChair(id, client);
        getHaircut(id, client);
        pay(id, client);
        // 1 indica que executou a rotina com sucesso
        leaveShop(id, 1);
    } else {
        // 0 indica falha em entrar
        leaveShop(id, 0);
    }

    // Recursos do cliente sao liberados
    freeClientSemaphores(client);

    return NULL;
}

// Cria e retorna uma thread do cliente com o id fornecido
pthread_t createClientThread(unsigned long id) {
    pthread_t thr;
    pthread_create(&thr, NULL, clientThread, (void*)id);

    return thr;
}

// Inicializa os semaforos da struct Client e retorna esse cliente
Client* initClientSemaphores() {
    Client* client = malloc(sizeof(Client));
    client->shopToClient_sem = malloc(sizeof(sem_t));
    sem_init(client->shopToClient_sem, 0, 0);
    client->clientToShop_sem = malloc(sizeof(sem_t));
    sem_init(client->clientToShop_sem, 0, 0);

    return client;
}

// Libera os semaforos do cliente e o cliente
void freeClientSemaphores(Client* client) {
    free(client->clientToShop_sem);
    free(client->shopToClient_sem);

    free(client);
}

// Cliente - Funcionamento

// Entra na loja e fica de pe na fila para sentar no sofa
void enterShop(unsigned long id, Client* client) {
    // Mutex para se colocar na fila para sentar no sofa
    pthread_mutex_lock(&standupToSofa_mutex);
    enQueue(standupToSofaQueue, client);
    pthread_mutex_unlock(&standupToSofa_mutex);

    printf("Cliente %ld esta esperando de pe\n", id);
}

// O cliente aguarda por espaco no sofa e entao senta
// e entra na fila para uma cadeira para cortar cabelo
void waitAndSeatOnSofa(unsigned long id, Client* client) {
    printf("Cliente %ld esta esperando por um lugar no sofa\n", id);
    // Avisa que esta aguardando pelo sofa
    sem_post(&clientWaitingForSofa_sem);
    // Espera que a secretaria avise que pode sentar
    sem_wait(client->shopToClient_sem);

    printf("Cliente %ld esta sentando no sofa\n", id);
    // Mutex para entrar na fila de espera para cortar o cabelo
    pthread_mutex_lock(&sofaToBarber_mutex);
    sleep(SEAT_TIME);
    enQueue(sofaToBarberQueue, client);
    pthread_mutex_unlock(&sofaToBarber_mutex);
}

// O cliente avisa que esta esperando e aguarda ser chamado
// para cortar cabelo
void seatOnBarberChair(unsigned long id, Client* client) {
    sem_post(&clientWaitingForBarberChair_sem);
    sem_wait(client->shopToClient_sem);
    printf("Cliente %ld esta sentando na cadeira para cortar\n", id);
    sleep(SEAT_TIME);
}

// Cliente avisa que esta pronto para cortar e espera o
// sinal do barbeiro de que terminou
void getHaircut(unsigned long id, Client* client) {
    sem_post(client->clientToShop_sem);
    sem_wait(client->shopToClient_sem);
}

// Cliente avisa que quer pagar e aguarda um barbeiro receber
void pay(unsigned long id, Client* client) {
    sem_post(client->clientToShop_sem);
    printf("Cliente %ld esta esperando para pagar\n", id);
    sem_wait(client->shopToClient_sem);
    printf("Cliente %ld terminou de pagar\n", id);
}

// Cliente sai da loja e libera o espaco de capacidade,
// caso tenha conseguido entrar e cortar o cabelo
void leaveShop(unsigned long id, short int success) {
    if (success) {
        printf("Cliente %ld saiu apos cortar o cabelo\n", id);
        sem_post(&capacity_sem);
    } else {
        printf("Cliente %ld saiu sem cortar o cabelo\n", id);
    }
}

// Rotina de criacao de threads
void doThreadsCreationAndJoins(Parameters params) {
    // Cria as threads de barbeiros
    pthread_t barbers[params.N_BARBERS];
    for (size_t i = 0; i < params.N_BARBERS; i++) {
        barbers[i] = createBarberThread(i);
    }

    // Cria a thread de secretaria
    pthread_t secretaria;
    secretaria = createSecretariaThread(0);

    // Cria as threads de clientes
    pthread_t clients[params.N_CLIENTS];
    for (size_t i = 0; i < params.N_CLIENTS; i++) {
        clients[i] = createClientThread(i);
    }

    // Join nas threads de clientes
    for (size_t i = 0; i < params.N_CLIENTS; i++) {
        pthread_join(clients[i], NULL);
    }

    // ! Joins na secretaria e nos barbeiros desativados
    // para que o programa apenas finalize !

    // Join na thread de secretaria
    // pthread_join(secretaria, NULL);

    // // Join nas threads de barbeiros
    // for (size_t i = 0; i < params.N_BARBERS; i++) {
    //     pthread_join(barbers[i], NULL);
    // }
}