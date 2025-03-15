/* proxy-mq.c
 * Implementa las funciones de la API (destroy, set_value, get_value, modify_value, delete_key, exist)
 * en el lado del cliente, encapsulando la comunicación mediante colas de mensajes POSIX.
 * Estas funciones se compilan en una biblioteca dinámica (libclaves.so) para que la aplicación cliente
 * invoque la API sin conocer los detalles de la comunicación.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include "claves.h"

// Definición de nombres para las colas de mensajes
#define REQUEST_QUEUE "/mq_cliente"   // Cola para enviar peticiones (el servidor la lee)
#define RESPONSE_QUEUE "/mq_servidor"   // Cola para recibir respuestas (el servidor la escribe)

// Estructura que se utiliza para empaquetar las peticiones al servidor.
struct Request {
    int op_type;         // Código de operación: 1: set_value, 2: get_value, 3: modify_value, 4: exist, 5: delete_key, 6: destroy
    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
};

// Nueva estructura para la respuesta completa en get_value
struct Response {
    int result;              // Código de resultado: 0, -1, -2, etc.
    char value1[256];        // Cadena recuperada
    int N_value2;            // Número de elementos del vector
    double V_value2[32];     // Vector de doubles
    struct Coord value3;     // Estructura de coordenadas
};

// Función auxiliar para operaciones que sólo necesitan transmitir un entero
static int send_request(struct Request *req) {
    mqd_t mq_req, mq_resp;

    // Abrir la cola de peticiones en modo escritura.
    mq_req = mq_open(REQUEST_QUEUE, O_WRONLY);
    if (mq_req == (mqd_t)-1) {
        perror("proxy: error al abrir la cola de peticiones");
        return -2;  // Código de error de comunicación
    }

    // Abrir la cola de respuestas en modo lectura.
    mq_resp = mq_open(RESPONSE_QUEUE, O_RDONLY);
    if (mq_resp == (mqd_t)-1) {
        perror("proxy: error al abrir la cola de respuestas");
        mq_close(mq_req);
        return -2;
    }

    // Enviar la petición.
    if (mq_send(mq_req, (char *)req, sizeof(struct Request), 0) == -1) {
        perror("proxy: error al enviar la petición");
        mq_close(mq_req);
        mq_close(mq_resp);
        return -2;
    }
    mq_close(mq_req);

    // Usar un buffer suficientemente grande para recibir la respuesta,
    // ya que la cola se creó con tamaño maximo sizeof(struct Response)
    char buffer[sizeof(struct Response)];
    if (mq_receive(mq_resp, buffer, sizeof(buffer), NULL) == -1) {
        perror("proxy: error al recibir la respuesta");
        mq_close(mq_resp);
        return -2;
    }
    mq_close(mq_resp);
    
    // Extraer el entero de la respuesta (se asume que para estas operaciones, el primer campo es el resultado)
    int result = *((int *)buffer);
    return result;
}

// Función auxiliar para get_value que espera una respuesta completa
static int send_request_get(struct Request *req, struct Response *resp) {
    mqd_t mq_req, mq_resp;

    mq_req = mq_open(REQUEST_QUEUE, O_WRONLY);
    if (mq_req == (mqd_t)-1) {
        perror("proxy (get): error al abrir la cola de peticiones");
        return -2;
    }

    mq_resp = mq_open(RESPONSE_QUEUE, O_RDONLY);
    if (mq_resp == (mqd_t)-1) {
        perror("proxy (get): error al abrir la cola de respuestas");
        mq_close(mq_req);
        return -2;
    }

    if (mq_send(mq_req, (char *)req, sizeof(struct Request), 0) == -1) {
        perror("proxy (get): error al enviar la petición");
        mq_close(mq_req);
        mq_close(mq_resp);
        return -2;
    }
    mq_close(mq_req);

    if (mq_receive(mq_resp, (char *)resp, sizeof(struct Response), NULL) == -1) {
        perror("proxy (get): error al recibir la respuesta");
        mq_close(mq_resp);
        return -2;
    }
    mq_close(mq_resp);

    return resp->result;
}

// Implementación de las funciones de la API:

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    struct Request req;
    memset(&req, 0, sizeof(req));
    req.op_type = 1;  // set_value
    req.key = key;
    strncpy(req.value1, value1, 255);
    req.value1[255] = '\0';
    req.N_value2 = N_value2;
    memcpy(req.V_value2, V_value2, sizeof(double) * N_value2);
    req.value3 = value3;
    return send_request(&req);
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    struct Request req;
    memset(&req, 0, sizeof(req));
    req.op_type = 2;  // get_value
    req.key = key;
    
    struct Response resp;
    memset(&resp, 0, sizeof(resp));
    
    int result = send_request_get(&req, &resp);
    if (result == 0) {
        strncpy(value1, resp.value1, 256);
        *N_value2 = resp.N_value2;
        memcpy(V_value2, resp.V_value2, sizeof(double) * resp.N_value2);
        *value3 = resp.value3;
    }
    return result;
}

int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    struct Request req;
    memset(&req, 0, sizeof(req));
    req.op_type = 3;  // modify_value
    req.key = key;
    strncpy(req.value1, value1, 255);
    req.value1[255] = '\0';
    req.N_value2 = N_value2;
    memcpy(req.V_value2, V_value2, sizeof(double) * N_value2);
    req.value3 = value3;
    return send_request(&req);
}

int exist(int key) {
    struct Request req;
    memset(&req, 0, sizeof(req));
    req.op_type = 4;  // exist
    req.key = key;
    return send_request(&req);
}

int delete_key(int key) {
    struct Request req;
    memset(&req, 0, sizeof(req));
    req.op_type = 5;  // delete_key
    req.key = key;
    return send_request(&req);
}

int destroy(void) {
    struct Request req;
    memset(&req, 0, sizeof(req));
    req.op_type = 6;  // destroy
    return send_request(&req);
}
