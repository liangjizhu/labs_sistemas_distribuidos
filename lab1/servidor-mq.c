/* Servidor-mq.c
 * Servidor encargado de las comunicaciones con la parte cliente.
 * Debe ser concurrente y procesar las peticiones invocando las funciones de claves.c.
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
    int op_type; // tipo de operación
    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
};

// Nueva estructura para la respuesta completa de get_value
struct Response {
    int result;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
};

int init_queues() {
    struct mq_attr attr_req, attr_resp;

    mq_unlink("/mq_cliente");
    mq_unlink("/mq_servidor");

    // Cola de peticiones (Request)
    attr_req.mq_flags = 0;
    attr_req.mq_maxmsg = 10;
    attr_req.mq_msgsize = sizeof(struct Request);
    attr_req.mq_curmsgs = 0;
    mq_cliente = mq_open("/mq_cliente", O_CREAT | O_RDONLY, 0644, &attr_req);
    if (mq_cliente == (mqd_t) -1) {
        perror("Error al abrir la cola de cliente");
        return -1;
    }

    // Cola de respuestas: para operaciones get_value enviaremos struct Response,
    // para las demás un int. Como máximo se envía el mayor tamaño de ambos:
    int max_size = (sizeof(struct Response) > sizeof(int)) ? sizeof(struct Response) : sizeof(int);
    attr_resp.mq_flags = 0;
    attr_resp.mq_maxmsg = 10;
    attr_resp.mq_msgsize = max_size;
    attr_resp.mq_curmsgs = 0;
    mq_servidor = mq_open("/mq_servidor", O_CREAT | O_WRONLY, 0644, &attr_resp);
    if (mq_servidor == (mqd_t) -1) {
        perror("Error al abrir la cola del servidor");
        return -1;
    }

    return 0;
}

void* conc_clientes(void* arg) {
    struct Request *request = (struct Request*) arg;
    printf("Mensaje recibido: operation: %d, key: %d\n", request->op_type, request->key);

    int result = -1;
    if (request->op_type != 2) {
        // Para operaciones que solo requieren enviar un entero de respuesta
        switch(request->op_type) {
            case 1:
                result = set_value(request->key, request->value1, request->N_value2, request->V_value2, request->value3);
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
        // Enviar respuesta (int)
        if (mq_send(mq_servidor, (char*)&result, sizeof(int), 0) == -1) {
            perror("Error sending response to client");
        }
    } else {
        // Caso get_value: preparamos una respuesta completa
        char tmp_value1[256];
        int tmp_N;
        double tmp_V[32];
        struct Coord tmp_coord;
        result = get_value(request->key, tmp_value1, &tmp_N, tmp_V, &tmp_coord);

        struct Response resp;
        resp.result = result;
        if (result == 0) {
            strncpy(resp.value1, tmp_value1, 256);
            resp.N_value2 = tmp_N;
            memcpy(resp.V_value2, tmp_V, sizeof(double)*tmp_N);
            resp.value3 = tmp_coord;
        }
        if (mq_send(mq_servidor, (char*)&resp, sizeof(struct Response), 0) == -1) {
            perror("Error sending response (get_value) to client");
        }
    }

    free(request);
    return NULL;
}

int main() {
    if (init_queues() == -1) {
        return -1;
    }

    printf("Servidor iniciado. Esperando peticiones de clientes...\n");

    while (1) {
        struct Request request;
        if (mq_receive(mq_cliente, (char*)&request, sizeof(struct Request), NULL) == -1) {
            perror("Error recibiendo mensaje de cliente");
            continue;
        }

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
 