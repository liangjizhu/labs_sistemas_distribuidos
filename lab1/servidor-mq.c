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
    int op_type; // tipo de operacion
    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
};

// Iniciar colas
int init_queues() {
    struct mq_attr attr_req, attr_resp;

    // Eliminar colas existentes (si las hay)
    mq_unlink("/mq_cliente");
    mq_unlink("/mq_servidor");

    // Configuración para la cola de peticiones (Request)
    attr_req.mq_flags = 0;
    attr_req.mq_maxmsg = 10;
    attr_req.mq_msgsize = sizeof(struct Request);
    attr_req.mq_curmsgs = 0;
    mq_cliente = mq_open("/mq_cliente", O_CREAT | O_RDONLY, 0644, &attr_req);
    if (mq_cliente == (mqd_t) -1) {
        perror("Error al abrir la cola de cliente");
        return -1;
    }

    // Configuración para la cola de respuestas (int)
    attr_resp.mq_flags = 0;
    attr_resp.mq_maxmsg = 10;
    attr_resp.mq_msgsize = sizeof(int);
    attr_resp.mq_curmsgs = 0;
    mq_servidor = mq_open("/mq_servidor", O_CREAT | O_WRONLY, 0644, &attr_resp);
    if (mq_servidor == (mqd_t) -1) {
        perror("Error al abrir la cola del servidor");
        return -1;
    }

    return 0;  // Éxito
}





void* conc_clientes(void* arg) {
    // Utilizar el mensaje ya recibido que se pasa como argumento
    struct Request *request = (struct Request*) arg;

    printf("Mensaje recibido: operation: %d, key: %d\n", request->op_type, request->key);

    int result = -1;
    switch(request->op_type) {
        case 1:
            result = set_value(request->key, request->value1, request->N_value2, request->V_value2, request->value3);
            break;
        case 2:
            result = get_value(request->key, request->value1, &request->N_value2, request->V_value2, &request->value3);
            break;
        case 3:
            result = modify_value(request->key, request->value1, request->N_value2, request->V_value2, request->value3);
            break;
        case 4:
            result = exist(request->key);
            break;
        case 5:
            result = delete_key(request->key);
            break;
        case 6:
            result = destroy();
            break;
        default:
            printf("Operación desconocida\n");
            break;
    }

    // Enviar la respuesta al cliente
    if (mq_send(mq_servidor, (char*)&result, sizeof(int), 0) == -1) {
        perror("Error sending response to client");
    }

    free(request); // Liberar la memoria asignada en main
    return NULL;
}


int main() {
    if (init_queues() == -1) {
        return -1;
    }

    printf("Servidor iniciado. Esperando peticiones de clientes...\n");

    while (1) {
        struct Request request;
        // Esperar de forma bloqueante a que llegue un mensaje
        if (mq_receive(mq_cliente, (char*)&request, sizeof(struct Request), NULL) == -1) {
            perror("Error recibiendo mensaje de cliente");
            continue;
        }

        // Crear un bloque de memoria para almacenar la petición
        struct Request *req_ptr = malloc(sizeof(struct Request));
        if (!req_ptr) {
            perror("Error al asignar memoria para la petición");
            continue;
        }
        memcpy(req_ptr, &request, sizeof(struct Request));

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, conc_clientes, req_ptr) != 0) {
            perror("Error al crear hilo");
            free(req_ptr);
            continue;
        }
        pthread_detach(thread_id);
    }

    mq_close(mq_cliente);
    mq_close(mq_servidor);
    mq_unlink("/mq_cliente");
    mq_unlink("/mq_servidor");

    return 0;
}
