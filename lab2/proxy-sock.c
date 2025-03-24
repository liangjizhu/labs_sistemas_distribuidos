#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "claves.h"

#define OP_SET_VALUE 1

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    int sd;
    struct sockaddr_in server_addr;
    char *ip = getenv("IP_TUPLAS");
    char *port_str = getenv("PORT_TUPLAS");
    int port;

    if (!ip || !port_str) {
        fprintf(stderr, "Error: IP_TUPLAS o PORT_TUPLAS no están definidas\n");
        return -1;
    }

    port = atoi(port_str);

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

    if (connect(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sd);
        return -1;
    }

    char op_code = OP_SET_VALUE;
    send(sd, &op_code, sizeof(char), 0);

    // 1. Enviar key
    int32_t key_net = htonl(key);
    send(sd, &key_net, sizeof(int32_t), 0);

    // 2. Enviar value1
    int32_t len1 = htonl(strlen(value1));
    send(sd, &len1, sizeof(int32_t), 0);
    send(sd, value1, strlen(value1), 0);

    // 3. Enviar N_value2
    int32_t nval2_net = htonl(N_value2);
    send(sd, &nval2_net, sizeof(int32_t), 0);

    // 4. Enviar V_value2
    for (int i = 0; i < N_value2; i++) {
        uint64_t val_net;
        memcpy(&val_net, &V_value2[i], sizeof(double));
        val_net = htobe64(val_net);
        send(sd, &val_net, sizeof(uint64_t), 0);
    }

    // 5. Enviar value3
    int32_t x_net = htonl(value3.x);
    int32_t y_net = htonl(value3.y);
    send(sd, &x_net, sizeof(int32_t), 0);
    send(sd, &y_net, sizeof(int32_t), 0);

    // Recibir respuesta
    char status;
    if (recv(sd, &status, sizeof(char), 0) <= 0) {
        fprintf(stderr, "Error al recibir la respuesta del servidor\n");
        close(sd);
        return -1;
    }

    close(sd);
    return (status == 0) ? 0 : -1;
}

