// Adaptado de
// https://www.geeksforgeeks.org/queue-linked-list-implementation/

#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

// Struct que guarda os semaforos dos clientes
typedef struct Client {
    sem_t *shopToClient_sem, // Comunicacao loja -> cliente
          *clientToShop_sem; // Comunicacao cliente -> loja
} Client;

// No para uso em lista ligada
struct QNode {
    Client* key;
    struct QNode* next;
};

// Struct da fila, front e a parte da frente e rear a parte de tras
struct Queue {
    struct QNode *front, *rear;
};

// Cria um novo no
struct QNode* newNode(Client* k) {
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->key = k;
    temp->next = NULL;
    return temp;
}

// Cria uma nova fila vazia
struct Queue* createQueue() {
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}

// Adiciona um cliente na fila
void enQueue(struct Queue* q, Client* k) {
    // Cria um no
    struct QNode* temp = newNode(k);

    // Se a fila estava vazia, o no representa a frente e tras
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }

    // Adiciona o no no final da fila e atualiza a parte de tras
    q->rear->next = temp;
    q->rear = temp;
}

// Remove um cliente da fila
void deQueue(struct Queue* q) {
    // Se a fila esta vazia nao faz nada
    if (q->front == NULL) return;

    // Guarda o no da frente e muda a frente para o proximo no
    struct QNode* temp = q->front;
    q->front = q->front->next;

    // Se a frente e nula, a fila esta vazia, entao tras e nulo
    if (q->front == NULL) q->rear = NULL;

    free(temp);
}