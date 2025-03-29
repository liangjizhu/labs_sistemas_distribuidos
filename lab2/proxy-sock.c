#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "claves.h"

#define OP_SET_VALUE 1
#define OP_GET_VALUE 2
#define OP_MODIGY_VALUE 3
#define OP_EXIST 4
#define OP_DELETE_KEY 5
#define OP_DESTROY 6

int connect_to_server() {
    int sd;
    struct sockaddr_in server_addr;
    char *ip = getenv("IP_TUPLAS");
    char *port_str = getenv("PORT_TUPLAS");
    
    if (!ip || !port_str) {
        fprintf(stderr, "Error: IP_TUPLAS o PORT_TUPLAS no están definidas\n");
        return -1;
    }
    
    int port = atoi(port_str);
    
    // Crear socket TCP
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sd);
        return -1;
    }
    
    // Conectar al servidor
    if (connect(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sd);
        return -1;
    }
    
    return sd;
}


int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    int sd = connect_to_server();
    if (sd < 0) {
        return -1;
    }

    char op_code = OP_SET_VALUE;
    send(sd, &op_code, sizeof(char), 0);

    // 1. Enviar key (int32_t en orden de red)
    int32_t key_net = htonl(key);
    if (send(sd, &key_net, sizeof(int32_t), 0) < 0) {
        perror("send key");
        close(sd);
        return -1;
    }

    // 2. Enviar value1: primero la longitud (int32_t), luego la cadena
    int32_t len = strlen(value1);
    int32_t len_net = htonl(len);
    if (send(sd, &len_net, sizeof(int32_t), 0) < 0) {
        perror("send value1 length");
        close(sd);
        return -1;
    }
    if (send(sd, value1, len, 0) < 0) {
        perror("send value1");
        close(sd);
        return -1;
    }

    // 3. Enviar N_value2 (int32_t)
    int32_t nval2_net = htonl(N_value2);
    if (send(sd, &nval2_net, sizeof(int32_t), 0) < 0) {
        perror("send N_value2");
        close(sd);
        return -1;
    }

    // 4. Enviar V_value2 (cada uno como un uint64_t convertido de double)
    for (int i = 0; i < N_value2; i++) {
        uint64_t val_net;
        memcpy(&val_net, &V_value2[i], sizeof(double));
        val_net = htobe64(val_net);
        if (send(sd, &val_net, sizeof(uint64_t), 0) < 0) {
            perror("send V_value2 element");
            close(sd);
            return -1;
        }
    }

    // 5. Enviar value3: enviar x y y (int32_t cada uno)
    int32_t x_net = htonl(value3.x);
    int32_t y_net = htonl(value3.y);
    if (send(sd, &x_net, sizeof(int32_t), 0) < 0) {
        perror("send value3.x");
        close(sd);
        return -1;
    }
    if (send(sd, &y_net, sizeof(int32_t), 0) < 0) {
        perror("send value3.y");
        close(sd);
        return -1;
    }

    // Recibir respuesta del servidor: se espera un status
    char status;
    if (recv(sd, &status, sizeof(char), 0) <= 0) {
        fprintf(stderr, "Error al recibir la respuesta del servidor\n");
        close(sd);
        return -1;
    }

    close(sd);
    return (status == 0) ? 0 : -1;
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    int sd = connect_to_server();
    if (sd < 0) {
        return -1;
    }

    char op_code = OP_GET_VALUE;
    if (send(sd, &op_code, sizeof(char), 0) < 0) {
        perror("send op_code");
        close(sd);
        return -1;
    }

    // Enviar la clave
    int32_t key_net = htonl(key);
    if (send(sd, &key_net, sizeof(int32_t), 0) < 0) {
        perror("send key");
        close(sd);
        return -1;
    }

    // Recibir status (char)
    char status;
    if (recv(sd, &status, sizeof(char), MSG_WAITALL) <= 0) {
        perror("receive status");
        close(sd);
        return -1;
    }
    if (status != 0) { // error en el servidor
        close(sd);
        return -1;
    }

    // 1. Recibir longitud de value1
    int32_t len_net;
    if (recv(sd, &len_net, sizeof(int32_t), MSG_WAITALL) <= 0) {
        perror("recv value1 length");
        close(sd);
        return -1;
    }
    int len = ntohl(len_net);
    
    // 2. Recibir la cadena value1
    if (recv(sd, value1, len, MSG_WAITALL) <= 0) {
        perror("recv value1");
        close(sd);
        return -1;
    }
    value1[len] = '\0';  // Asegurarse de terminar la cadena

    // 3. Recibir N_value2 (int32_t)
    int32_t nval2_net = htonl(N_value2);
    if (recv(sd, &nval2_net, sizeof(int32_t), MSG_WAITALL) <= 0) {
        perror("receive N_value2");
        close(sd);
        return -1;
    }
    *N_value2 = ntohl(nval2_net);

    // 4. Recibir cada elemento del vector V_value2
    for (int i = 0; i < *N_value2; i++) {
        uint64_t d_net;
        if (recv(sd, &d_net, sizeof(uint64_t), MSG_WAITALL) <= 0) {
            perror("recv V_value2 element");
            close(sd);
            return -1;
        }
        uint64_t d_host = be64toh(d_net);
        memcpy(&V_value2[i], &d_host, sizeof(double));
    }

    // 5. Recibir value3: dos int32_t
    int32_t x_net, y_net;
    if (recv(sd, &x_net, sizeof(int32_t), MSG_WAITALL) <= 0) {
        perror("recv value3.x");
        close(sd);
        return -1;
    }
    if (recv(sd, &y_net, sizeof(int32_t), MSG_WAITALL) <= 0) {
        perror("recv value3.y");
        close(sd);
        return -1;
    }
    value3->x = ntohl(x_net);
    value3->y = ntohl(y_net);

    close(sd);
    return 0;
}
