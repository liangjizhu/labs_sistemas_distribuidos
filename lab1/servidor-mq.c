/* Servidor encargado de las comunicaciones con la parte cliente
 * Debe de ser concurrente
 * Recibe las peticiones del cliente e invoca las funciones en claves.c
 */

#include <mqueue.h>
#include <fcntl.h> // O_CREAT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "claves.h"

mqd_t mq_cliente, mq_servidor;

struct Request {
    int op_type;
    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
};

int init_queues() {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(int);
    attr.mq_curmsgs = 0;

    mq_cliente = mq_open("/mq_cliente", O_CREAT | O_RDONLY, 0644, &attr);
    if (mq_cliente == (mqd_t) -1) {
        perror("Error al abrir la cola de cliente");
        return -1;
    }

    mq_servidor = mq_open("/mq_servidor", O_CREAT | O_WRONLY, 0644, &attr);
    if (mq_servidor == (mqd_t) -1) {
        perror("Error al abrir la cola del servidor");
        return -1;
    }

    return 0;  // Éxito
}


void* conc_clientes(void* arg) {
    struct Request request;

    // Se recibe mensaje de cliente
    if (mq_receive(mq_cliente, (char*)&request, sizeof(struct Request), NULL) == -1) {
        perror("Error recibiendo mensaje de cliente");
        return NULL;
    }

    printf("Mensaje recibido: operation: %d, key: %d\n", request.op_type, request.key);


    //
    int result = -1;
    switch(request.op_type) {
        case 1:
            result = set_value(request.key, request.value1, request.N_value2, request.V_value2, request.value3);
        case 2:
            result = get_value(request.key, request.value1, &request.N_value2, request.V_value2, &request.value3);
        case 3:
            result = modify_value(request.key, request.value1, request.N_value2, request.V_value2, request.value3);
        case 4:
            result = delete_key(request.key);
        default:
            printf("Operación no se reconoce");
            break;
    }

    //Enviar la respuesta al cliente
    if (mq_send(mq_servidor, (char*)&result, sizeof(int), 0) == -1) {
        perror("Error sending response to client");
    }

    return NULL;

}

int main() {
    //Inicializar cola de mensaje
    if (init_queues() == -1) {
        return -1;
    }

    printf("Servidor iniciado. Esperando respuesta de cliente...");

    while(1) {
        pthread_t thread_id;

        // Create a new thread to handle each client request
        if (pthread_create(&thread_id, NULL, conc_clientes, NULL) != 0) {
            perror("Error al crear hilo");
        }

        // Detach the thread to automatically clean up when finished
        pthread_detach(thread_id);
    }


    mq_close(mq_cliente);
    mq_close(mq_servidor);

    mq_unlink("/mq_cliente");
    mq_unlink("/mq_servidor");

    return 0;
}